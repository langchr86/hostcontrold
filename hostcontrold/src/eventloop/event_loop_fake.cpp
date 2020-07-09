#include "event_loop_fake.h"

#include "eventloop/internal/event_controller.h"

EventLoopFake::EventLoopFake() : controller_(std::make_shared<EventController>()) {

}

std::unique_ptr<Event> EventLoopFake::CreateOneShotTimer(TimerHandler&& handler, const Duration& delay) {

}

std::unique_ptr<Event> EventLoopFake::CreateRecurringTimer(TimerHandler&& handler, const Duration& interval) {

}
