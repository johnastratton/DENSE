//
//  in_memory_log_main.cpp
//
//
//  Created by Myan Sudharsanan on 7/2/20.
//
#ifndef LogTimes_main_h
#define LogTimes_main_h



#include <stdio.h>
#include "io/arg_parse.hpp"
#include "measurement/oscillation.hpp"
#include "measurement/basic.hpp"
#include "measurement/bad_simulation_error.hpp"
#include "utility/style.hpp"
#include "utility/common_utils.hpp"
#include "io/csvr_sim.hpp"
#include "io/csvw_sim.hpp"
#include "sim/determ/num_sim.hpp"
#include "sim/determ/trap.hpp"
#include "sim/stoch/fast_gillespie_direct_simulation.hpp"
#include "sim/stoch/next_reaction_simulation.hpp"
#include "model_impl.hpp"
#include "io/ezxml/ezxml.h"
#include "Sim_Builder.hpp"
#include "run_simulation.hpp"
#include "run_analysis_only.hpp"
#include "arg_parse.hpp"
#include "parse_analysis_entries.hpp"
#include "io/csvr.hpp"
#include "sim/base.hpp"
#include "core/specie.hpp"
#include "in_memory_log.hpp"
#include "Callback.hpp"
#include "runtimecheck.hpp"

using style::Color;
#include <chrono>
#include <cstdlib>
#include <cassert>
#include <random>
#include <memory>
#include <iterator>
#include <algorithm>
#include <functional>
#include <exception>
#include <iostream>
using dense::csvw_sim;
using dense::Trapezoid_Simulation;
using dense::Sim_Builder;
using dense::parse_static_args;
using dense::parse_analysis_entries;
using dense::Static_Args;
using dense::run_simulation;
int main(int argc, char* argv[]){
    int ac = argc;
    char** av = argv;
    Static_Args args = parse_static_args(argc, argv);
    if(args.help == 1){
        return EXIT_SUCCESS;
    }
    if(args.help == 2){
        return EXIT_FAILURE;
    }
    using Simulation = Deterministic_Simulation;
    if(args.param_sets.size() == 0){
        std::cout << style::apply(Color::red) << "param_sets is empty" << '\n';
        return 0;
    }
    Sim_Builder<Simulation> sim = Sim_Builder<Simulation>(args.perturbation_factors, args.gradient_factors, args.adj_graph, ac, av);

    std::vector<Species> species_list;
    #define SPECIE(name) species_list.push_back(name);
    #include "specie_list.hpp"
    #undef SPECIE

    std::vector<std::pair<std::string, std::unique_ptr<Analysis<Simulation>>>> buffer_analysis_form;

    buffer_analysis_form.emplace_back("", std14::make_unique<in_memory_log<Simulation> >(
            species_list, std::make_pair<dense::Natural, dense::Natural>(0, args.adj_graph.num_vertices()), args.analysis_interval));

    runtimecheck r;

    //vector of call backs or analysis, myans new function
    std::vector<Callback<Simulation> > in_memory_log_returns = run_simulation<Simulation>(args.simulation_duration, args.analysis_interval, sim.get_simulations(args.param_sets), std::move(buffer_analysis_form));

    r.set_end();
    r.set_begin();
    //convert above vector to get vector of inmemory log objects which are the analyses?
    std::vector<in_memory_log<Simulation>> new_buffer;
    for(unsigned int i = 0; i < in_memory_log_returns.size(); i++){
        new_buffer.emplace_back(std::move(*static_cast<in_memory_log<Simulation>*>(in_memory_log_returns[i].get_analysis().release())));
    }

    r.set_end();
    r.set_begin();

    std::vector<Callback<in_memory_log<Simulation> > > callbacks = run_simulation(args.simulation_duration, args.analysis_interval, std::move(new_buffer), parse_analysis_entries<in_memory_log<Simulation>>(argc, argv, args.adj_graph.num_vertices()));

    r.set_end();
    r.set_begin();

    for (auto& callback : callbacks) {
        callback.finalize();
        callback.show();
    }

    r.set_end();

    auto duration1 = r.get_duration(0, 0);
    std::cout << "First Simulation only: " << duration1 << endl;
    auto duration2 = r.get_duration(1, 1);
    std::cout << "Vector Conversion only: " << duration2 << endl;
    auto duration3 = r.get_duration(2, 2);
    std::cout << "Second Simulation only: " << duration3 << endl;
    auto duration4 = r.get_duration(3, 3);
    std::cout << "Finalization only: " << duration4 << endl;
    auto duration5 = r.get_duration(3, 0);
    std::cout << "Total duration: " << duration5 << endl;
}
#endif