// Singleplayer game template.
#define SINGLE_PLAYER

#include "math/math.cc"

#include "animation/sprite.cc"
#include "audio/audio.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
#include "renderer/imui.cc"

struct State {
  // Game and render updates per second
  u64 framerate = 60;
  // Calculated available microseconds per game_update
  u64 frame_target_usec;
  // Estimate of gime passed since game start.
  u64 game_time_usec = 0;
  // Estimated frames per second.
  r32 frames_per_second = 0;
  // Number of times the game has been updated.
  u64 game_updates = 0;
  // Parameters window::Create will be called with.
  window::CreateInfo window_create_info;
  // Id to music.
  u32 music_id;
};

static State kGameState;
static Stats kGameStats;

struct Asset {
  rgg::Texture player_texture;  
  animation::Sprite player_sprite;
};

static Asset kAsset;

struct Game {
  r32 ground;
};

static Game kGame;

struct PhysicsGlobal {
  float friction_coefficient = .05f;
  float gravity = 1.f;
};

static PhysicsGlobal kPhysicsGlobal;

struct PhysicsComponent {
  u32 id;
  v3f position;
  v3f velocity;
  v3f acceleration;
  v3f max_speed = v3f(1.f, 10.f, 0.f);
  Rectf bounds;
};

DECLARE_HASH_ARRAY(PhysicsComponent, 16);

struct RenderSpriteComponent {
  u32 id;
  rgg::Texture texture;
  animation::Sprite sprite_anim;
};

DECLARE_HASH_ARRAY(RenderSpriteComponent, 8);

struct RenderRectComponent {
  u32 id;
};

DECLARE_HASH_ARRAY(RenderRectComponent, 8);

struct Entity {
  u32 id;
  u64 physics_component_id = kInvalidId;
  u64 render_sprite_component_id = kInvalidId;
  u64 render_rect_component_id = kInvalidId;
};

DECLARE_HASH_ARRAY(Entity, 16);

static u32 kPlayerId = kInvalidId;

#define UIBUFFER_SIZE 64
static char kUIBuffer[UIBUFFER_SIZE];

void
DebugUI()
{
  v2f screen = window::GetWindowSize();
  {
    static b8 enable_debug = false;
    static v2f diagnostics_pos(3.f, screen.y);
    static r32 right_align = 130.f;
    imui::PaneOptions options;
    options.max_width = 315.f;
    imui::Begin("Diagnostics", imui::kEveryoneTag, options, &diagnostics_pos,
                &enable_debug);
    imui::TextOptions debug_options;
    debug_options.color = imui::kWhite;
    debug_options.highlight_color = imui::kRed;
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Frame Time");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%04.02fus [%02.02f%%]",
             StatsMean(&kGameStats), 100.f * StatsUnbiasedRsDev(&kGameStats));
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Game Time");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%04.02fs",
             (r64)kGameState.game_time_usec / 1e6);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("FPS");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%04.02ff/s",
             (r64)kGameState.game_updates /
                 ((r64)kGameState.game_time_usec / 1e6));
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Window Size");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%.0fx%.0f", screen.x, screen.y);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::End();
  }

  {
    static b8 enable_debug = false;
    static v2f ui_pos(300.f, screen.y);
    imui::DebugPane("UI Debug", imui::kEveryoneTag, &ui_pos, &enable_debug);
  }
}

void
InitializeGame()
{
  kGame.ground = 0.f;
  // Create the player.
  Entity* entity = UseEntity();
  PhysicsComponent* physics = UsePhysicsComponent();
  entity->physics_component_id = physics->id;
  RenderSpriteComponent* sprite_component = UseRenderSpriteComponent();
  sprite_component->texture = kAsset.player_texture;
  sprite_component->sprite_anim = kAsset.player_sprite;
  entity->render_sprite_component_id = sprite_component->id;
  physics->bounds = Rectf(0.f, 0.f, 20.f, 31.f);
  entity->render_rect_component_id = UseRenderRectComponent()->id;
}

b8
LoadAssets()
{
  rgg::TextureInfo tinfo;
  tinfo.mag_filter = GL_NEAREST;
  tinfo.min_filter = GL_NEAREST_MIPMAP_NEAREST;
  if (!rgg::LoadTGA("asset/adventurer.tga", tinfo, &kAsset.player_texture)) {
    return false;
  }

  if (!animation::LoadAnimation("asset/adventurer.anim", &kAsset.player_sprite)) {
    return false;
  }

  return true;
}

void
UpdateGame()
{
  rgg::CameraUpdate();

  for (int i = 0; i < kUsedEntity; ++i) {
    PhysicsComponent* physics =
        FindPhysicsComponent(kEntity[i].physics_component_id);

    if (!physics) continue;

    v3f pre_velocity = physics->velocity;
    physics->velocity += physics->acceleration;

    if (physics->velocity.x * physics->velocity.x >
        physics->max_speed.x * physics->max_speed.x) {
      if (physics->velocity.x < 0.f) physics->velocity.x = -physics->max_speed.x;
      if (physics->velocity.x > 0.f) physics->velocity.x = physics->max_speed.x;
    }

    physics->position += physics->velocity;
    if (physics->velocity.x > 0.f) {
      physics->velocity.x -= kPhysicsGlobal.friction_coefficient;
      if (physics->velocity.x < 0.f) physics->velocity.x = 0.f;
    } else if (physics->velocity.x < 0.f) {
      physics->velocity.x += kPhysicsGlobal.friction_coefficient;
      if (physics->velocity.x > 0.f) physics->velocity.x = 0.f;
    }

    if (physics->velocity.y > 0.f) {
      physics->velocity.y -= kPhysicsGlobal.gravity;
      if (physics->velocity.x < 0.f) physics->velocity.x = 0.f;
    }

    if (physics->position.y > kGame.ground) {
      physics->velocity.y -= kPhysicsGlobal.gravity;
    }

    if (physics->position.y < kGame.ground) {
      physics->velocity.y = 0.f;
      physics->position.y = kGame.ground;
    }

    physics->bounds.x = physics->position.x;
    physics->bounds.y = physics->position.y;

    RenderSpriteComponent* sprite =
        FindRenderSpriteComponent(kEntity[i].render_sprite_component_id);

    if (!sprite) continue;

    // TODO: Is this actually right?
    physics->bounds.x += ((float)sprite->sprite_anim.width / 4.f) + 2.f;

    if (pre_velocity.x <= 0.f && physics->velocity.x > 0.f) {
      animation::SetLabel("run", &sprite->sprite_anim);
    } else if (pre_velocity.x >= 0.f && physics->velocity.x < 0.f) {
      animation::SetLabel("run", &sprite->sprite_anim, true);
    }

    if (pre_velocity.x > 0.f && physics->velocity.x == 0.f) {
      animation::SetLabel("idle", &sprite->sprite_anim);
    }

    if (pre_velocity.x < 0.f && physics->velocity.x == 0.f) {
      animation::SetLabel("idle", &sprite->sprite_anim, true);
    }
  }
}

void
RenderGame()
{
  rgg::Observer* obs = rgg::GetObserver();
  obs->view = rgg::CameraView();

  rgg::RenderLine(v3f(-1000.f, kGame.ground, 0.f),
                  v3f(1000.f, kGame.ground, 0.f),
                  v4f(1.f, 1.f, 1.f, 1.f));

  // Render all rect components.
  for (u32 i = 0; i < kUsedEntity; ++i) {
    Entity* ent = &kEntity[i];

    PhysicsComponent* physics =
        FindPhysicsComponent(ent->physics_component_id);

    if (!physics) continue;

    RenderRectComponent* rect =
        FindRenderRectComponent(ent->render_rect_component_id);

    if (!rect) continue;

    rgg::RenderLineRectangle(physics->bounds, 0.f, v4f(1.f, 0.f, 0.f, 1.f));
  }

  // Render all sprite components.
  for (u32 i = 0; i < kUsedEntity; ++i) {
    Entity* ent = &kEntity[i];

    PhysicsComponent* physics =
        FindPhysicsComponent(ent->physics_component_id);

    if (!physics) continue;

    RenderSpriteComponent* sprite =
        FindRenderSpriteComponent(ent->render_sprite_component_id);

    if (!sprite) continue;

    rgg::RenderTexture(
        sprite->texture, animation::Update(&sprite->sprite_anim),
        Rectf(physics->position.xy(),
              sprite->sprite_anim.width, sprite->sprite_anim.height),
        sprite->sprite_anim.mirror);
  }
  
  rgg::DebugRenderPrimitives();
  imui::Render(imui::kEveryoneTag);

  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

s32
main(s32 argc, char** argv)
{
  platform::Clock game_clock;

  if (!memory::Initialize(MiB(64))) {
    return 1;
  }

  if (!window::Create("Platformer", kGameState.window_create_info)) {
    return 1;
  }

  if (!rgg::Initialize()) {
    return 1;
  }

  if (!audio::Initialize()) {
    printf("Unable to initialize audio system.\n");
    return 1;
  }

  if (!LoadAssets()) {
    printf("Unable to load assets.\n");
    return 1;
  }

  InitializeGame();

  rgg::GetObserver()->projection = rgg::DefaultOrtho(window::GetWindowSize());

  rgg::Camera camera;
  camera.position = v3f(0.f, 1.f, -.69f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraFollow;
  camera.speed = v3f(5.f, 5.f, 0.1f);
  rgg::CameraInit(camera);

  // main thread affinity set to core 0
  if (platform::thread_affinity_count() > 1) {
    platform::thread_affinity_usecore(0);
    printf("Game thread may run on %d cores\n",
           platform::thread_affinity_count());
  }

  // Reset State
  StatsInit(&kGameStats);
  kGameState.game_updates = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
  printf("Client target usec %lu\n", kGameState.frame_target_usec);

  // If vsync is enabled, force the clock_init to align with clock_sync
  // TODO: We should also enforce framerate is equal to refresh rate
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  while (1) {
    platform::ClockStart(&game_clock);

    imui::ResetTag(imui::kEveryoneTag);
    rgg::DebugReset();

    if (window::ShouldClose()) break;

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      rgg::CameraUpdateEvent(event);
      Entity* player = FindEntity(kPlayerId);
      PhysicsComponent* physics =
          FindPhysicsComponent(player->physics_component_id);
      switch(event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 'w': {
              physics->acceleration.y = 3.f;
            } break;
            case 'a': {
              physics->acceleration.x = -1.f;
            } break;
            case 's': {
            } break;
            case 'd': {
              physics->acceleration.x = 1.f;
            } break;
            case 27 /* ESC */: {
              exit(1);
            } break;
          }
        } break;
        case KEY_UP: {
          switch (event.key) {
            case 'w': {
              physics->acceleration.y = 0.f;
            } break;
            case 'a': {
              physics->acceleration.x = 0.f;
            } break;
            case 'd': {
              physics->acceleration.x = 0.f;
            } break;
          }
        } break;

        case MOUSE_DOWN: {
          imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
        } break;
        case MOUSE_UP: {
          imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
        } break;
        case MOUSE_WHEEL: {
          imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
        } break;
      }
    }

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);

    DebugUI();

    UpdateGame();
    RenderGame();
    
    const u64 elapsed_usec = platform::ClockEnd(&game_clock);
    StatsAdd(elapsed_usec, &kGameStats);

    if (kGameState.frame_target_usec > elapsed_usec) {
      u64 wait_usec = kGameState.frame_target_usec - elapsed_usec;
      platform::Clock wait_clock;
      platform::ClockStart(&wait_clock);
      while (platform::ClockEnd(&wait_clock) < wait_usec) {}
    }

    kGameState.game_time_usec += platform::ClockEnd(&game_clock);
    kGameState.game_updates++;
  }

  return 0;
}