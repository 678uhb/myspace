
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN
/*********************
 *
 *   params of energy vad
 *
 * **********************/
class EnergyVadParam {
public:
  float &decisionThreshold() { return decision_threshold_; }
  float &contaminationRejectionPeriod() {
    return contamination_rejection_period_;
  }
  float &onsetDectectDur() { return onset_detect_dur_; }
  float &onsetWindow() { return onset_window_; }
  float &onsetConfirmDur() { return onset_confirm_dur_; }
  float &speechOnWindow() { return speech_on_window_; }
  float &onMaintainDur() { return on_maintain_dur_; }
  float &offsetWindow() { return offset_window_; }
  float &minDecisionThreshold() { return min_decision_threshold_; }
  float &framePeriod() { return frame_period_; }
  float &offsetConfirmDur() { return offset_confirm_dur_; }
  float &fastUpdateDur() { return fast_update_dur_; }
  size_t &sampleRate() { return sample_rate_; }

private:
  size_t sample_rate_ = 16000;
  // Initial rms detection threshold
  float decision_threshold_ = 100.0f;
  // Period after start of user input that above threshold values are ignored.
  // This is to reject audio feedback contamination.
  float contamination_rejection_period_ = 0.25f;
  // Total dur within onset_window required to enter ONSET state
  float onset_detect_dur_ = 0.09f;
  // Interval scanned for onset activity
  float onset_window_ = 0.15f;
  // Total on time within onset_window required to enter SPEECH_ON state
  float onset_confirm_dur_ = 0.075f;
  // Inverval scanned for ongoing speech
  float speech_on_window_ = 0.4f;
  // Minimum dur in SPEECH_ON state required to maintain ON state
  float on_maintain_dur_ = 0.10f;
  // Interval scanned for offset evidence
  float offset_window_ = 0.15f;
  // Minimum rms detection threshold
  float min_decision_threshold_ = 50.0f;
  // Frame period
  float frame_period_ = 0.02f;
  // Silence duration required to confirm offset
  float offset_confirm_dur_ = 0.12f;
  // Period for initial estimation of levels.
  float fast_update_dur_ = 0.2f;
};
/*********************
 *
 *   energy vad handle
 *
 * **********************/
class EnergyVad {
public:
  // Endpointer status codes
  enum EpStatus {
    EP_PRE_SPEECH = 10,
    EP_POSSIBLE_ONSET,
    EP_SPEECH_PRESENT,
    EP_POSSIBLE_OFFSET,
    EP_POST_SPEECH,
  };

public:
  EnergyVad(const EnergyVadParam &param);
  EnergyVad::EpStatus process(const int16_t *voice, size_t length);
  EnergyVad::EpStatus process(const std::string &voice);

private:
  float RMS(const int16_t *samples, size_t num_samples);
  void updateState(int64_t time_us, float rms);
  int timeToFrame(float time);
  int64_t Secs2Usecs(float seconds) {
    return static_cast<int64_t>(0.5 + (1.0e6 * seconds));
  }
  void adapt(bool decision, float rms);
  void updateLevels(float rms);

private:
  EpStatus status_ = EP_PRE_SPEECH;
  class HistoryRing;
  std::unique_ptr<HistoryRing> history_;
  EnergyVadParam params_;
  bool estimating_environment_ = false;
  // Time of the most recently received audio frame.
  int64_t endpointer_time_us_ = 0;
  // Time when mode switched from environment estimation to user input. This
  // is used to time forced rejection of audio feedback contamination.
  int64_t user_input_start_time_us_ = 0;
  // Largest search window size (seconds)
  float max_window_dur_ = 4.0;
  // RMS which must be exceeded to conclude frame is speech.
  float decision_threshold_;
  // An adaptive threshold used to update decision_threshold_ when appropriate.
  float rms_adapt_;
  // Estimate of the background noise level. Used externally for UI feedback.
  float noise_level_;
  // Number of frames seen. Used for initial adaptation.
  int64_t frame_counter_ = 0;
  // max on time allowed to confirm POST_SPEECH
  float offset_confirm_dur_sec_;
  // Number of frames for initial level adaptation.
  int64_t fast_update_frames_;
};

class EnergyVad::HistoryRing {
public:
  // Resets the ring to |size| elements each with state |initial_state|
  void setRing(int size, bool initial_state) {
    insertion_index_ = 0;
    decision_points_.clear();
    DecisionPoint init;
    init.time_us = -1;
    init.decision = initial_state;
    decision_points_.resize(size, init);
  }

  // Inserts a new entry into the ring and drops the oldest entry.
  void insert(int64_t time_us, bool decision) {
    decision_points_[insertion_index_].time_us = time_us;
    decision_points_[insertion_index_].decision = decision;
    insertion_index_ = (insertion_index_ + 1) % decision_points_.size();
  }

  // Returns the time in microseconds of the most recently added entry.
  int64_t EndTime() const {
    size_t ind = insertion_index_ - 1;
    if (insertion_index_ == 0)
      ind = decision_points_.size() - 1;
    return decision_points_[ind].time_us;
  }

  // Returns the sum of all intervals during which 'decision' is true within
  // the time in seconds specified by 'duration'. The returned interval is
  // in seconds.
  float ringSum(float duration_sec) {
    if (!decision_points_.size())
      return 0.0;

    int64_t sum_us = 0;
    size_t ind = insertion_index_ - 1;
    if (insertion_index_ == 0)
      ind = decision_points_.size() - 1;
    int64_t end_us = decision_points_[ind].time_us;
    bool is_on = decision_points_[ind].decision;
    int64_t start_us =
        end_us - static_cast<int64_t>(0.5 + (1.0e6 * duration_sec));
    if (start_us < 0)
      start_us = 0;
    size_t n_summed = 1; // n points ==> (n-1) intervals
    while ((decision_points_[ind].time_us > start_us) &&
           (n_summed < decision_points_.size())) {
      if (ind > 0)
        --ind;
      else
        ind = decision_points_.size() - 1;
      if (is_on)
        sum_us += end_us - decision_points_[ind].time_us;
      is_on = decision_points_[ind].decision;
      end_us = decision_points_[ind].time_us;
      n_summed++;
    }

    return 1.0e-6f * sum_us; //  Returns total time that was super threshold.
  }

private:
  struct DecisionPoint {
    int64_t time_us = 0;
    bool decision = false;
  };
  std::deque<DecisionPoint> decision_points_;
  size_t insertion_index_ =
      0; // Index at which the next item gets added/inserted.
};

inline int EnergyVad::timeToFrame(float time) {
  return static_cast<int32_t>(0.5 + (time / params_.framePeriod()));
}

inline EnergyVad::EnergyVad(const EnergyVadParam &param) : params_(param) {

  history_ = std::make_unique<HistoryRing>();

  max_window_dur_ = params_.onsetWindow();
  if (max_window_dur_ < params_.speechOnWindow())
    max_window_dur_ = params_.speechOnWindow();
  if (max_window_dur_ < params_.offsetWindow())
    max_window_dur_ = params_.offsetWindow();

  rms_adapt_ = decision_threshold_ = params_.decisionThreshold();
  noise_level_ = rms_adapt_ / 2.0f;

  history_->setRing(timeToFrame(max_window_dur_), false);
  // Flag that indicates that current input should be used for
  // estimating the environment. The user has not yet started input
  // by e.g. pressed the push-to-talk button. By default, this is
  // false for backward compatibility.
  estimating_environment_ = false;

  offset_confirm_dur_sec_ = params_.offsetWindow() - params_.offsetConfirmDur();

  fast_update_frames_ =
      static_cast<int64_t>(params_.fastUpdateDur() / params_.framePeriod());
}
inline EnergyVad::EpStatus EnergyVad::process(const std::string &voice) {
  return process((const int16_t *)voice.c_str(), voice.size() / 2);
}
inline EnergyVad::EpStatus EnergyVad::process(const int16_t *voice,
                                              size_t length) {
  endpointer_time_us_ += length * 1000000 / params_.sampleRate();
  auto rms = RMS(voice, length);
  if (!estimating_environment_) {
    updateState(endpointer_time_us_, rms);
  }
  // Update speech and noise levels.
  updateLevels(rms);
  ++frame_counter_;
  return status_;
}

inline void EnergyVad::updateLevels(float rms) {
  // Update quickly initially. We assume this is noise and that
  // speech is 6dB above the noise.
  if (frame_counter_ < fast_update_frames_) {
    // Alpha increases from 0 to (k-1)/k where k is the number of time
    // steps in the initial adaptation period.
    float alpha = static_cast<float>(frame_counter_) /
                  static_cast<float>(fast_update_frames_);
    noise_level_ = (alpha * noise_level_) + ((1 - alpha) * rms);
    // DVLOG(1) << "FAST UPDATE, frame_counter_ " << frame_counter << ",
    // fast_update_frames_ " << fast_update_frames_;
  } else {
    // Update Noise level. The noise level adapts quickly downward, but
    // slowly upward. The noise_level_ parameter is not currently used
    // for threshold adaptation. It is used for UI feedback.
    if (noise_level_ < rms)
      noise_level_ = (0.999f * noise_level_) + (0.001f * rms);
    else
      noise_level_ = (0.95f * noise_level_) + (0.05f * rms);
  }
  if (estimating_environment_ || (frame_counter_ < fast_update_frames_)) {
    decision_threshold_ = noise_level_ * 2; // 6dB above noise level.
    // Set a floor
    if (decision_threshold_ < params_.minDecisionThreshold())
      decision_threshold_ = params_.minDecisionThreshold();
  }
}

inline void EnergyVad::updateState(int64_t time_us, float rms) {
  bool decision = false;
  if ((endpointer_time_us_ - user_input_start_time_us_) <
      Secs2Usecs(params_.contaminationRejectionPeriod())) {
    decision = false;
  } else {
    decision = (rms > decision_threshold_);
  }
  history_->insert(time_us, decision);
  switch (status_) {
  case EP_PRE_SPEECH: {
    if (history_->ringSum(params_.onsetWindow()) > params_.onsetDectectDur()) {
      status_ = EP_POSSIBLE_ONSET;
    }
  } break;
  case EP_POSSIBLE_ONSET: {
    auto tsum = history_->ringSum(params_.onsetWindow());
    if (tsum > params_.onsetConfirmDur()) {
      status_ = EP_SPEECH_PRESENT;
    } else if (tsum < params_.onsetDectectDur()) {
      status_ = EP_PRE_SPEECH;
    }
  } break;
  case EP_SPEECH_PRESENT: {
    // To induce hysteresis in the state residency, we allow a
    // smaller residency time in the on_ring, than was required to
    // enter the SPEECH_PERSENT state.
    float on_time = history_->ringSum(params_.speechOnWindow());
    if (on_time < params_.onMaintainDur())
      status_ = EP_POSSIBLE_OFFSET;
  } break;
  case EP_POSSIBLE_OFFSET:
    if (history_->ringSum(params_.offsetWindow()) <= offset_confirm_dur_sec_) {
      // Note that this offset time may be beyond the end
      // of the input buffer in a real-time system.  It will be up
      // to the RecognizerSession to decide what to do.
      status_ = EP_PRE_SPEECH; // Automatically reset for next utterance.
    } else { // If speech picks up again we allow return to SPEECH_PRESENT.
      if (history_->ringSum(params_.speechOnWindow()) >=
          params_.onMaintainDur())
        status_ = EP_SPEECH_PRESENT;
    }
    break;
  }
  adapt(decision, rms);
}

inline void EnergyVad::adapt(bool decision, float rms) {
  // If this is a quiet, non-speech region, slowly adapt the detection
  // threshold to be about 6dB above the average RMS.
  if ((!decision) && (status_ == EP_PRE_SPEECH)) {
    decision_threshold_ = (0.98f * decision_threshold_) + (0.02f * 2 * rms);
    rms_adapt_ = decision_threshold_;
  } else {
    // If this is in a speech region, adapt the decision threshold to
    // be about 10dB below the average RMS. If the noise level is high,
    // the threshold is pushed up.
    // Adaptation up to a higher level is 5 times faster than decay to
    // a lower level.
    if ((status_ == EP_SPEECH_PRESENT) && decision) {
      if (rms_adapt_ > rms) {
        rms_adapt_ = (0.99f * rms_adapt_) + (0.01f * rms);
      } else {
        rms_adapt_ = (0.95f * rms_adapt_) + (0.05f * rms);
      }
      float target_threshold = 0.3f * rms_adapt_ + noise_level_;
      decision_threshold_ =
          (.90f * decision_threshold_) + (0.10f * target_threshold);
    }
  }

  // Set a floor
  if (decision_threshold_ < params_.minDecisionThreshold())
    decision_threshold_ = params_.minDecisionThreshold();
}

inline float EnergyVad::RMS(const int16_t *samples, size_t num_samples) {
  int64_t ssq_int64 = 0;
  int64_t sum_int64 = 0;
  for (size_t i = 0; i < num_samples; ++i) {
    sum_int64 += samples[i];
    ssq_int64 += samples[i] * samples[i];
  }
  // now convert to floats.
  double sum = static_cast<double>(sum_int64);
  sum /= num_samples;
  double ssq = static_cast<double>(ssq_int64);
  return static_cast<float>(std::sqrt((ssq / num_samples) - (sum * sum)));
}

MYSPACE_END
