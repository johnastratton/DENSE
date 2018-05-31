#ifndef CORE_MODEL_HPP
#define CORE_MODEL_HPP

#include <cstdlib>
#include <iostream>

#include "reaction.hpp"
#include "specie.hpp"


class model{
public:
    model() {}

    static delay_reaction_id getDelayReactionId(reaction_id rid) {
        switch (rid) {
            #define REACTION(name)
            #define DELAY_REACTION(name) \
            case name : return dreact_##name;
            #include "reactions_list.hpp"
            #undef REACTION
            #undef DELAY_REACTION
            default: return NUM_DELAY_REACTIONS;
        }
    }

    reaction_base const& getReaction(reaction_id rid) const {
        switch (rid) {
            #define REACTION(name) \
            case name: return reaction_##name;
            #include "reactions_list.hpp"
            #undef REACTION
            default: std::cout<<"exiting"<<'\n'; std::exit(-1);
        }
    }

    #define REACTION(name) reaction<name> reaction_##name;
    #include "reactions_list.hpp"
    #undef REACTION
};

#endif
