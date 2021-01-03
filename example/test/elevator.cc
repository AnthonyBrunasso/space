// Elevator

// Consider -

//   Latency    : Optimize for average speed of delivered occupants.
//   Throughput : Optimize for number of occupants delivered per min.
//   Starvation : Don't let any outstanding requests go unserviced.

// Where should the elevator stop?

//  Queue of requests
//  Algorithm for latency...
//    if elevator has no occupants - go to the most recent request
//    otherwise - go to where the occupants have chosen the elevator to go

// Optimizing for throughput
//    while the elevator is going up / or down if it passes floors with requests
//    that are also going up or down it can stop for them.

// Where should the elevator go next?
struct Request {
  bool picked_up = false;
  int from_floor;
  int to_floor;
};

class Elevator {
 public: 
  Request* request(); // Find first request that requires the user be picked up
  Request* current_request();

  int current_floor() const;

  bool has_destination() const { return destination_ != -1; }

 private:
  Queue<Request> requests_;
  Request* current_request_ = nullptr;
};

void Request(Elevator* elevator, int from_floor, int to_floor) {
  Request request;
  request.from_floor = from_floor;
  request.to_floor = to_floor;
  request.picked_up = false;
  elevator->request_.Push(request);
}

// For Throughput: Pickup people on the way to your destination
//                 and update the queue of requests / swapping them
//                 to the front???

void GoNext(Elevator* elevator) {
  // Has a pickup request and not servicing a request
  if (elevator->request() && !elevator->current_request()) {
    elevator->current_request_ = &elevator->request();
  }

  if (elevator->current_request() && !elevator->current_request()->picked_up) {
    elevator->GoTo(elevator->current_request()->from_floor);
  }

  if (elevator->current_request() &&
      elevator->current_floor() == elevator->current_request().from_floor) {
    elevator->current_request()->picked_up = true;
  }

  if (elevator->current_request() && elevator->picked_up) {
    elevator->GoTo(elevator->current_request()->to_floor); 
  }

  // Go through my queue and see if I happen to be sitting at a floor that
  // has a corresponding request that is also going in the direction of the
  // elevator. move it to the front, the percolate it back through requests
  // who have been picked up if my drop off comes after it. Update current
  // reuqest if it no longer matches whats at the top of the queue....

  if (elevator->current_request() &&
      elevator->current_floor() == elevator->current_request().to_floor) {
    elevator->requests_.Pop();
    elevator->current_request_ = nullptr;
  }
}
