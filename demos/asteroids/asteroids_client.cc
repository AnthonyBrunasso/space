#include <thread>
#include <gflags/gflags.h>

#include "asteroids.h"
#include "asteroids_commands.h"
#include "asteroids_state.h"
#include "components/common/transform_component.h"
#include "components/common/input_component.h"
#include "network/client.h"
#include "network/message_queue.h"
#include "protocol/asteroids_commands_generated.h"
#include "game/game.h"

#include "ecs/internal.h"

DEFINE_string(hostname, "",
              "If provided will connect to a game server. Will play "
              "the game singleplayer otherwise.");
DEFINE_string(port, "9845", "Port for this application.");

bool IsSinglePlayer() { return FLAGS_hostname.empty(); }

void ProcessIncomingCommands() {
  // Execute all commands received from server.
  auto* connection_component =
      asteroids::GlobalGameState().singleton_components
          .Get<asteroids::ConnectionComponent>();
  network::Message msg =
      connection_component->incoming_message_queue.Dequeue();
  while (msg.size != 0) {
    asteroids::commands::Execute(msg.data);
    msg = connection_component->incoming_message_queue.Dequeue();
  }
}

void ProcessOutgoingCommands() {
  auto& components = asteroids::GlobalGameState().components;
  auto* connection_component =
      asteroids::GlobalGameState().singleton_components
          .Get<asteroids::ConnectionComponent>();
  components.Enumerate<component::ClientAuthoratativeComponent<
      component::InputComponent>>([&](
          ecs::Entity entity, component::ClientAuthoratativeComponent<
              component::InputComponent>){
    auto* input_component =
        components.Get<component::InputComponent>(entity);
    assert(input_component != nullptr);
    // TODO: Make automatic conversions for some of these things...
    flatbuffers::FlatBufferBuilder fbb;
    asteroids::UpdateInput update_input(
        entity,
        asteroids::Input(input_component->input_mask,
                        input_component->previous_input_mask));
    auto create_command =
        asteroids::CreateCommand(fbb, 0, 0, 0, 0, 0, 0, &update_input);
    fbb.Finish(create_command);
    connection_component->outgoing_message_queue.Enqueue(
        fbb.Release());
  });
}

void ProcessClientInput() {
  auto& opengl = asteroids::GlobalOpenGL();
  auto& components = asteroids::GlobalGameState().components;
  glfwPollEvents();
  components.Enumerate<component::InputComponent>(
      [&opengl](ecs::Entity ent, component::InputComponent& input) {
    input.previous_input_mask = input.input_mask;
    input.input_mask = 0;
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_W)) {
      component::SetKeyDown(input.input_mask, component::KEYBOARD_W); 
    }
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_A)) {
      component::SetKeyDown(input.input_mask, component::KEYBOARD_A); 
    }
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_S)) {
      component::SetKeyDown(input.input_mask, component::KEYBOARD_S); 
    }
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_D)) {
      component::SetKeyDown(input.input_mask, component::KEYBOARD_D); 
    }
    if (glfwGetKey(opengl.glfw_window, GLFW_KEY_SPACE)) {
      component::SetKeyDown(input.input_mask,
                            component::KEYBOARD_SPACE); 
    }
  });
}

class AsteroidsClient : public game::Game {
 public:
  AsteroidsClient() : game::Game() {}

  bool Initialize() override {
    auto* connection_component =
        asteroids::GlobalGameState().singleton_components
            .Get<asteroids::ConnectionComponent>();
    if (!FLAGS_hostname.empty()) {
      connection_component->network_thread = network::client::Create(
          FLAGS_hostname.c_str(), FLAGS_port.c_str(),
          &connection_component->outgoing_message_queue,
          &connection_component->incoming_message_queue);
      connection_component->is_client = true;
    }

    // Clients entities start at max free entity and then decrement.
    // These are largely ephemeral and will be deleted when the server
    // replicates client created entities.
    asteroids::SetEntityStart(ecs::ENTITY_LAST_FREE);
    asteroids::SetEntityIncrement(-1);

    // Add a single receipient for the server. Otherwise messages
    // can not be added to the queue :(
    connection_component->outgoing_message_queue.AddRecipient(0);
    if (!asteroids::Initialize()) {
      std::cout << "Failed to initialize asteroids." << std::endl;
      return false;
    }

    asteroids::CreatePlayer create_player(
        asteroids::GenerateFreeEntity(),
        asteroids::Vec3(0.f, 0.f, 0.f));
    asteroids::commands::Execute(create_player);
   
    if (IsSinglePlayer()) {
      asteroids::GlobalGameState().components
          .Assign<asteroids::GameStateComponent>(
              asteroids::GenerateFreeEntity());
    }
  
    return true;
  }

  bool ProcessInput() override {
    ProcessClientInput();
    ProcessIncomingCommands();
    ProcessOutgoingCommands();
    return true;
  }

  bool Update() override {
    asteroids::UpdateGame();
    return true;
  }

  bool Render() override {
    return asteroids::RenderGame();
  }

  void OnGameEnd() override {
    auto* connection_component =
      asteroids::GlobalGameState().singleton_components
          .Get<asteroids::ConnectionComponent>();
    if (connection_component->network_thread.joinable()) {
      connection_component->outgoing_message_queue.Stop();
      connection_component->network_thread.join();
    }
  }
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  AsteroidsClient asteroids_client;
  asteroids_client.Run();
  return 0;
}
