
#include "ecos/scenario/scenario.hpp"

#include <spdlog/spdlog.h>

void ecos::scenario::apply(double t)
{

    active_ = true;

    for (auto& q : timedActionsQueue_) {
        timedActions.insert(std::upper_bound(timedActions.begin(), timedActions.end(), q), std::move(q));
    }
    timedActionsQueue_.clear();

    while (!timedActions.empty()) {
        auto& action = timedActions.back();
        const double nextT = action.time_point();
        double diff = std::abs(t - nextT);
        if (nextT < t || diff < action.eps()) {
            action.invoke();
            discardedTimedActions.emplace_back(std::move(action));
            timedActions.pop_back();
            spdlog::debug("Invoked timed action at t={}", t);
        } else {
            break;
        }
    }

    if (!predicateActions.empty()) {
        auto it = predicateActions.begin();
        while (it != predicateActions.end()) {
            if (it->invoke()) {
                discardedPredicateActions.emplace_back(std::move(*it));
                it = predicateActions.erase(it);
                spdlog::debug("Invoked predicate action at t={}", t);
            }
        }
    }

    active_ = false;
}
