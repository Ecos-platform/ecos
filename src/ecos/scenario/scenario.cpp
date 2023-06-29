
#include "ecos/scenario/scenario.hpp"

#include "ecos/logger/logger.hpp"

using namespace ecos;

void scenario::runInitActions()
{
    for (auto& a : initActions) {
        a();
    }
}

void scenario::on_init(std::function<void()> f)
{
    initActions.emplace_back(std::move(f));
}


void scenario::apply(double t)
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
            log::debug("Invoked timed action at t={:.3f}", t);
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
                log::debug("Invoked predicate action at t={:.3f}", t);
            }
        }
    }

    active_ = false;
}

void scenario::invoke_at(timed_action ta)
{
    if (!active_) {
        timedActions.insert(std::upper_bound(timedActions.begin(), timedActions.end(), ta), std::move(ta));
    } else {
        timedActionsQueue_.emplace_back(std::move(ta));
    }
}

void scenario::invoke_when(predicate_action pa)
{
    predicateActions.emplace_back(std::move(pa));
}

void scenario::reset()
{
    for (auto& a : discardedTimedActions) {
        timedActions.emplace_back(std::move(a));
    }
    for (auto& a : discardedPredicateActions) {
        predicateActions.emplace_back(std::move(a));
    }
}
