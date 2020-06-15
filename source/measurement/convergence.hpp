#ifndef ANLYS_CONVERGENCE_HPP
#define ANLYS_CONVERGENCE_HPP

#include "base.hpp"

#include <vector>
#include <stdexcept>
#include <cmath>

/*
* Subclass of Analysis superclass
* - records a predicted convergence vlaue
* - records if the data points converge or not
*/

template <typename Simulation>
class ConvergenceAnalysis : public Analysis<Simulation>{
  public:
    //structure for converges, convergence_values, convergence_start
    struct Convergence {
        Real value;
        Real start;
        void merge_results(Real newValue, Real newStart) {
            if (isnan(value)) {
                start = newStart;
            }
          value = newValue;
        }
    };


    ConvergenceAnalysis(Real interval, Real windowSize, Real threshold, std::vector<Species> const& pcfSpecieOption, 
                        std::pair<dense::Natural, dense::Natural> cell_range, 
                        std::pair<Real, Real> time_range = { 0, std::numeric_limits<Real>::infinity() }) :
        Analysis<Simulation>(pcfSpecieOption, cell_range, time_range), 
        window_size(windowSize), window_steps(windowSize/interval), 
        threshold(threshold), finalized(false), 
        converges(Analysis<>::observed_species_.size(), false)
    {
        auto cell_count = Analysis<>::max - Analysis<>::min;
        for (dense::Natural i = 0; i < cell_count; ++i) {
            windows.emplace_back(Analysis<>::observed_species_.size(), Queue<Real>(window_steps));
            convergences.emplace_back(Analysis<>::observed_species_.size(), Convergence{NAN, 0.0});
        }
    }

    //whenever the object has new data to be read
    //add concentration to queue
    //check for
    void update(Simulation& simulation, std::ostream&) override {
        
        //not sure yet how this type of update would differ from the other analysis types
        for (Natural c = this->min; c < this->max; ++c){
            for (std::size_t i =0; i < Analysis<>::observed_species_.size(); ++i){
              Real concentration = simulation.get_concentration(c, Analysis<>::observed_species_[i]);

              windows[c][i].enqueue(concentration);

              if(windows[c][i].getSize() == window_steps + 1){
                windows[c][i].dequeue();
              }
            
              convergences[c][i].merge_results(check_convergence(windows[c][i], concentration),
                  simulation.age()/Minutes(1));
            }
        }
    }

    Real check_convergence(const Queue<Real>& window, Real asymptote) {
      if ( window.getSize() != window_steps ) {
        return NAN;
      }

      for (dense::Natural i = 0; i < window.getSize(); i++) {
        if (abs( (window.getVal(i)/asymptote) - 1.0 ) > threshold) {
          return NAN;
        }
      }

      return asymptote;
    }

    
    

    //copy constructor
    ConvergenceAnalysis* clone() const override{
        return new auto(*this);
    }

    //call once your done collecting data
    void finalize() override {
        if(!finalized){
            finalized = true;
        }
        for (std::size_t i =0; i < Analysis<>::observed_species_.size(); ++i){
            converges[i] = true;
            for (Natural c = this->min; c < this->max; ++c){
                if (isnan(convergences[c][i].value)) {
                    converges[i] = false;
                    break;
                }
            }
        }
    }

    void show (csvw *csv_out = nullptr) override{
        Analysis<>::show(csv_out);
        if(csv_out){
            for (Natural c = this->min; c < this->max; ++c) {

                *csv_out << "\n# Showing cell " << c << "\nSpecies";
                 for (specie_id const& lcfID : this->observed_species_)
                     *csv_out << ',' << specie_str[lcfID];
                
                csv_out->add_div("\nconvergence value,");
                for(std::size_t i =0; i < Analysis<>::observed_species_.size(); ++i)
                    csv_out->add_data(convergences[c][i].value);
                

            }
        }
    }


    //analysis overview
    //what we give back to the user
    //must provide where convergence happens and when
    Details get_details() override {
        Details detail;
        for (std::size_t i =0; i < Analysis<>::observed_species_.size(); ++i){
            detail.concs.push_back( converges[i] ? 1.0 : 0.0 );
        }
        for (Natural c = this->min; c < this->max; ++c){
            std::vector<Real> values;
            std::vector<Real> starts;
            for (std::size_t i =0; Analysis<>::observed_species_.size(); ++i){
              values.push_back(convergences[c][i].value);
              starts.push_back(convergences[c][i].start);
            }
            detail.other_details.push_back(values);
            detail.other_details.push_back(starts);
        }
        return detail;
    }

  private:
    std::vector<std::vector<Queue<Real>>> windows;
    std::vector<std::vector<Convergence>> convergences;
    Real window_size;
    dense::Natural window_steps;
    Real threshold;
    bool finalized;
    std::vector<bool> converges;
};

#endif