#ifndef HMM_H
#define HMM_H


#include "fastlib/fastlib.h"

#include "distribution.h"

class HMM {

 private:

  int n_states_;
  int n_dims_;
  int T_;

  Vector p_initial_;

  Matrix p_transition_;

  ArrayList<Distribution> state_distributions_;

  Matrix state_probabilities_;

  Matrix cumulative_p_transition_;
  Matrix state_cumulative_probabilities_;
  
    

 public:

  void Init(int n_states_in, int n_dims_in, int T_in) {
    n_states_ = n_states_in;
    n_dims_ = n_dims_in;
    T_ = T_in;
    
    p_initial_.Init(n_states_);
    p_transition_.Init(n_states_, n_states_);
    state_distributions_.Init(n_states_);
    for(int i = 0; i < n_states_; i++) {
      state_distributions_[i].Init(n_dims_in);
    }

    state_probabilities_.Init(n_states_, T_);

    state_cumulative_probabilities_.Init(n_states_, T_);

    cumulative_p_transition_.Init(n_states_, n_states_);
    
  }

  void RandomlyInitialize() {
    double uniform = ((double) 1) / ((double) n_states_);

    for(int i = 0; i < n_states_; i++) {
      p_initial_[i] = uniform;
      
      for(int j = 0; j < n_states_; j++) {
	p_transition_.set(j, i, uniform);
      }

      state_distributions_[i].RandomlyInitialize();
    }
  }

  void CustomInitialize() {
    
    // set initial state probabilities
    double uniform = ((double) 1) / ((double) n_states_);

    double sum_p_initial = 0;

    for(int i = 0; i < n_states_; i++) {
      p_initial_[i] = 1.0 / ((double)(i+1));
      sum_p_initial += p_initial_[i];
    }

    for(int i = 0; i < n_states_; i++) {
      p_initial_[i] /= sum_p_initial;
    }


    // set state transition probabilities
    for(int j = 0; j < n_states_; j++) {
      for(int i = 0; i < n_states_; i++) {
	p_transition_.set(i, j, uniform);
      }
      p_transition_.set(j, j, 1.0);
    }

    for(int i = 0; i < n_states_; i++) {
      double sum = 0;
      for(int j = 0; j < n_states_; j++) {
	sum += p_transition_.get(i, j);
      }
      for(int j = 0; j < n_states_; j++) {
	p_transition_.set(i, j, p_transition_.get(i, j) / sum);
      }
    }
	
    // set state distributions
    for(int i = 0; i < n_states_; i++) {
      state_distributions_[i].RandomlyInitialize();
    }

    ComputeCumulativePTransition();

  }

  void ComputeCumulativePTransition() {

    for(int i = 0; i < n_states_; i++) {
      double cumsum = 0;
      for(int j = 0; j < n_states_; j++) {
	cumsum += p_transition_.get(i, j);
	cumulative_p_transition_.set(j, i, cumsum);
      }
    }
  }
    

  int n_states() {
    return n_states_;
  }

  int n_dims() {
    return n_dims_;
  }

  int T() {
    return T_;
  }
  
  Vector p_initial() {
    return p_initial_;
  }

  Matrix p_transition() {
    return p_transition_;
  }

  ArrayList<Distribution> state_distributions() {
    return state_distributions_;
  }

  Matrix state_probabilities() {
    return state_probabilities_;
  }

  Matrix state_cumulative_probabilities() {
    return state_cumulative_probabilities_;
  }

  Matrix cumulative_p_transition() {
    return cumulative_p_transition_;
  }
  
  void PrintDebug(const char *name = "", FILE *stream = stderr) /*const*/ {
    fprintf(stream, "----- HMM %s ------\n", name);
    
    p_initial_.PrintDebug("initial probabilities", stream);
    p_transition_.PrintDebug("transition probabilities", stream);
    
    for(int i = 0 ;i < n_states_; i++) {
      fprintf(stream, "state %d:\n", i+1);
      state_distributions_[i].mu().PrintDebug("mu");
      state_distributions_[i].sigma().PrintDebug("sigma");
      fprintf(stream, "\n");
    }
  }
  
  /* this function calculates P(q_t = s_i | theta) */
  void ComputeStateProbabilities() {
    
    // base case
    for(int j = 0; j < n_states_; j++) {
      state_probabilities_.set(j, 0, p_initial_[j]);
    }
    
    // recursive step
    for(int t = 1; t < T_; t++) {
      for(int j = 0; j < n_states_; j++) {
	double sum = 0;
	for(int i = 0; i < n_states_; i++) {
	  sum += state_probabilities_.get(i, t - 1) * p_transition_.get(i, j);
	}
	state_probabilities_.set(j, t, sum);
      }
    }


    // set state cumulative probabilities for efficient state draws
    for(int t = 0; t < T_; t++) {
      double cumsum = 0;
      for(int i = 0; i < n_states_; i++) {
	cumsum += state_probabilities_.get(i, t);
	state_cumulative_probabilities_.set(i, t, cumsum);
      }
    }
  }

  /**
   * Draw state from P(q_t)
   */
  int DrawState(int t) {
    double rand_num = drand48();

    int i = 0;
    while(rand_num > state_cumulative_probabilities_.get(i, t)) {
      i++;
    }
    
    return i;
  }

  /**
   * Draw state from P(q_t | q_{t-1} = s_i)
   */
  int DrawStateGivenLastState(int i) {
    double rand_num= drand48();

    int j = 0;
    while(rand_num > cumulative_p_transition_.get(j, i)) {
      j++;
    }

    return j;
  }

};


#endif /* HMM */
