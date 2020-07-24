#include <iostream>
#include <string>
#include <utility>

#include "constraint.hpp"

struct BackPack {
    std::string name;
    int money;
    int water;
    int apple;
    int chocolate;
    unsigned long hash; // This is important
public:
    BackPack(std::string _name, int _money, int _water, int _apple, int _chocolate) : name(std::move(_name)), money(_money), water(_water),
                                                                                      apple(_apple),
                                                                                      chocolate(_chocolate), hash(std::hash<std::string>()(_name)) {}

    // Defining these is necessary
    bool operator==(const BackPack &other) const {
        return this->name == other.name;
    }

    bool operator!=(const BackPack &other) const {
        return this->name != other.name;
    }

    bool operator<(const BackPack &other) const {
        return this->name < other.name;
    }
};

class MoneyConstraint : public FunctionConstraint<BackPack> {
    static constexpr int fix_money_needed = 100;
public:
    std::unique_ptr<Constraint<BackPack>> clone() override;

    [[nodiscard]] bool func(const std::vector<int> &parms, const std::vector<Proxy<BackPack>> &vars) const override;
};

std::unique_ptr<Constraint<BackPack>> MoneyConstraint::clone() {
    return std::make_unique<MoneyConstraint>(*this);
}

bool MoneyConstraint::func(const std::vector<int> &parms, const std::vector<Proxy<BackPack>> &vars) const {
    int total_money = 0;
    int total_water = 0;
    for (size_t i = 0; i < parms.size(); ++i) {
        if (parms[i] == 1) {
            total_money += vars[i].t_pointer->money;
            total_water += vars[i].t_pointer->water;
        }
    }
    int total_money_needed = fix_money_needed + total_water * 10;

    return total_money >= total_money_needed;

}

class WaterConstraint : public FunctionConstraint<BackPack> {
    static constexpr int water_needed = 5;
public:
    std::unique_ptr<Constraint<BackPack>> clone() override;

    [[nodiscard]] bool func(const std::vector<int> &parms, const std::vector<Proxy<BackPack>> &vars) const override;
};

std::unique_ptr<Constraint<BackPack>> WaterConstraint::clone() {
    return std::make_unique<WaterConstraint>(*this);
}

bool WaterConstraint::func(const std::vector<int> &parms, const std::vector<Proxy<BackPack>> &vars) const {
    int total_water = 0;
    for (size_t i = 0; i < parms.size(); ++i) {
        if (parms[i] == 1) {
            total_water += vars[i].t_pointer->water;
        }
    }
    return total_water >= water_needed;
}

class ChocolateConstraint : public FunctionConstraint<BackPack> {
    static constexpr int chocolate_needed = 3;
public:
    std::unique_ptr<Constraint<BackPack>> clone() override;

    [[nodiscard]] bool func(const std::vector<int> &parms, const std::vector<Proxy<BackPack>> &vars) const override;
};

std::unique_ptr<Constraint<BackPack>> ChocolateConstraint::clone() {
    return std::make_unique<ChocolateConstraint>(*this);
}

bool ChocolateConstraint::func(const std::vector<int> &parms, const std::vector<Proxy<BackPack>> &vars) const {
    int total_chocolate = 0;
    for (size_t i = 0; i < parms.size(); ++i) {
        if (parms[i] == 1) {
            total_chocolate += vars[i].t_pointer->chocolate;
        }
    }
    return total_chocolate >= chocolate_needed;
}

class AppleConstraint : public FunctionConstraint<BackPack> {
public:
    std::unique_ptr<Constraint<BackPack>> clone() override;

    [[nodiscard]] bool func(const std::vector<int> &parms, const std::vector<Proxy<BackPack>> &vars) const override;
};

std::unique_ptr<Constraint<BackPack>> AppleConstraint::clone() {
    return std::make_unique<AppleConstraint>(*this);
}

bool AppleConstraint::func(const std::vector<int> &parms, const std::vector<Proxy<BackPack>> &vars) const {
    int total_chocolate = 0;
    int total_apple = 0;
    for (size_t i = 0; i < parms.size(); ++i) {
        if (parms[i] == 1) {
            total_chocolate += vars[i].t_pointer->chocolate;
            total_apple += vars[i].t_pointer->apple;
        }
    }
    return total_apple >= total_chocolate; // Theoretically this constraint couldn't have been merged to the previous one
}

int main() {
    // A simple usecase example
    // Toy Scenario: A Roadtrip.
    // We can take as many backpack with us as we want. Each backpack contains certain items which can be useful
    // Our goal is to take as few bags as possible, while making sure we got everything
    // For every different item we have certain limits
    // Items Types: Money, Water, Apple, Chocolate
    // Constraint 1: At least 100 money + 10 additional money for every liter of water
    // Constraint 2: At least 5 liters water
    // Constraint 3: At least 3 chocolate
    // Constraint 4: At least as many apples as chocolate
    // We also assign a random name to each bag to keep track of them


    // Given these constraints, we can model them using the cpp-constraint library

    //First lets create our main variables, which are backpacks
    std::vector<BackPack> backpacks;
    backpacks.emplace_back("Red", 80, 0, 0, 0);
    backpacks.emplace_back("Blue", 50, 2, 3, 1);
    backpacks.emplace_back("Green", 35, 7, 1, 8);
    backpacks.emplace_back("Orange", 45, 3, 3, 3);
    backpacks.emplace_back("White", 20, 0, 5, 5);
    backpacks.emplace_back("Black", 50, 6, 6, 1);
    // Second lets built our constraints
    std::unique_ptr<Constraint<BackPack>> money_constraint = std::make_unique<MoneyConstraint>();
    std::unique_ptr<Constraint<BackPack>> water_constraint = std::make_unique<WaterConstraint>();
    std::unique_ptr<Constraint<BackPack>> chocolate_constraint = std::make_unique<ChocolateConstraint>();
    std::unique_ptr<Constraint<BackPack>> apple_constraint = std::make_unique<AppleConstraint>();

    // With these two ready, we can start defining our problem
    Problem<BackPack> problem;
    problem.add_variables(backpacks, {0, 1});
    problem.add_constraint(std::move(money_constraint), backpacks);
    problem.add_constraint(std::move(water_constraint), backpacks);
    problem.add_constraint(std::move(chocolate_constraint), backpacks);
    problem.add_constraint(std::move(apple_constraint), backpacks);

    // Now the problem is defined, we can solve it
    auto solutions = problem.get_solutions();
    std::cout << solutions.size() << std::endl;


    // Finally we can print some sample solutions
    for (size_t i = 0; i < 10; ++i) {
        if (i >= solutions.size()) {
            break;
        }
        std::string out;
        for (const auto &item : solutions[i]) {
            if (item.second == 1) {
                out += item.first.name + " \n";
            }
        }
        std::cout << out << std::endl;

    }

    return 0;
}
