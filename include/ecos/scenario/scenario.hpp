
#ifndef LIBECOS_SCENARIO_HPP
#define LIBECOS_SCENARIO_HPP

#include <algorithm>
#include <filesystem>
#include <functional>
#include <optional>
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
    timed_action(double timePoint, std::function<void()> f, const std::optional<double>& eps = std::nullopt)
        : f_(std::move(f))
        , eps_(eps.value_or(0))
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
    void runInitActions();

    void apply(double t);

    void on_init(std::function<void()> f);

    void invoke_when(predicate_action pa);

    void invoke_at(timed_action ta);

    void reset();


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
