// SensorPredictor.cpp
#include "CrossingModelT.h"
#include "modelXT.h"

CrossingPredictorT::CrossingPredictorT()
    : error_reporter(nullptr), model(nullptr), interpreter(nullptr), input(nullptr), output(nullptr) {}

bool CrossingPredictorT::initialize() {
    
    // Set up TensorFlow Lite
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "Model schema version %d not supported. Supported: %d",
                             model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "Tensor allocation failed");
        return false;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    return true;
}

CrossingType CrossingPredictorT::predictCrossingT(const PredictionData& SensorData){
    // Normalize and scale inputs
    float total_rgb = SensorData.red + SensorData.green + SensorData.blue;
    float red_ratio = SensorData.red / total_rgb;
    float green_ratio = SensorData.green / total_rgb;
    float blue_ratio = SensorData.blue / total_rgb;

    input->data.f[0] = (red_ratio - redR_mean) / redR_std;
    input->data.f[1] = (green_ratio - greenR_mean) / greenR_std;
    input->data.f[2] = (blue_ratio - blueR_mean) / blueR_std;

    // Perform inference
    if (interpreter->Invoke() != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "Inference failed");
        return CROSSING_T1; // Default fallback
    }

    float t1_score = output->data.f[0];
    float t2_score = output->data.f[1];
    float t3_score = output->data.f[2];

    if (t1_score > t2_score && t1_score > t3_score) {
        return CROSSING_T1;
    } else if (t2_score > t1_score && t2_score > t3_score) {
        return CROSSING_T2;
    } else {
        return CROSSING_T3;
    }
}
