
#ifndef LIBECOS_SCENARIO_HPP
#define LIBECOS_SCENARIO_HPP

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace ecos
{

class predicate_action
{

public:
    predicate_action(std::function<bool()> pred, std::function<void()> f)
        : f_(std::move(f))
        , pred_(std::move(pred))
    { }

    bool invoke()
    {
        bool shouldInvoke = pred_();
        if (shouldInvoke) {
            f_();
        }
        return shouldInvoke;
    }

private:
    std::function<bool()> pred_;
    std::function<void()> f_;
};

class timed_action
{

public:
    timed_action(std::function<void()> f, double timePoint, double eps = 0)
        : f_(std::move(f))
        , eps_(eps)
        , timePoint_(timePoint)
    { }

    [[nodiscard]] double eps() const
    {
        return eps_;
    }


    [[nodiscard]] double time_point() const
    {
        return timePoint_;
    }

    void invoke()
    {
        f_();
    }

    bool operator<(const timed_action& t) const
    {
        return timePoint_ > t.timePoint_;
    }

private:
    double eps_;
    double timePoint_;
    std::function<void()> f_;
};

class scenario
{

public:
    void runInitActions()
    {
        for (auto& a : initActions) {
            a();
        }
    }

    void apply(double t)
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
                }
            }
        }

        active_ = false;
    }

    void on_init(std::function<void()> f)
    {
        initActions.emplace_back(std::move(f));
    }

    void invoke_when(predicate_action pa)
    {
        predicateActions.emplace_back(std::move(pa));
    }

    void invoke_at(timed_action ta)
    {
        if (!active_) {
            timedActions.insert(std::upper_bound(timedActions.begin(), timedActions.end(), ta), std::move(ta));
        } else {
            timedActionsQueue_.emplace_back(std::move(ta));
        }
    }

    void reset()
    {
        for (auto& a : discardedTimedActions) {
            timedActions.emplace_back(std::move(a));
        }
        for (auto& a : discardedPredicateActions) {
            predicateActions.emplace_back(std::move(a));
        }
    }


private:
    bool active_;
    std::vector<std::function<void()>> initActions;
    std::vector<timed_action> timedActions;
    std::vector<predicate_action> predicateActions;

    std::vector<timed_action> discardedTimedActions;
    std::vector<predicate_action> discardedPredicateActions;

    std::vector<timed_action> timedActionsQueue_;
};

} // namespace ecos

#endif // LIBECOS_SCENARIO_HPP
