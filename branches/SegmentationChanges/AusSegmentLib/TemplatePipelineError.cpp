/*
 * TemplatePipelineError.cpp
 * Anita
 */
#include "TemplatePipelineError.h"

struct IrisErrCategory: std::error_category {
	const char* name() const noexcept override;
	std::string message(int errvalue) const override;
};

const char* IrisErrCategory::name() const noexcept
{
	return "IrisError";
}

std::string IrisErrCategory::message(int errvalue) const {
	switch (static_cast<TemplatePipelineError>(errvalue)) {
	case TemplatePipelineError::Pupil_Iris_Boundary_Not_Found:
		return "Pupil_Iris_Boundary_Not_Found";

	case TemplatePipelineError::Iris_Sclera_Boundary_Not_Found:
		return "Iris_Sclera_Boundary_Not_Found";

	case TemplatePipelineError::Usable_Iris_Area_not_sufficient:
		return "Usable_Iris_Area_not_sufficient";

	case TemplatePipelineError::Iris_Diameter_out_of_range:
		return "Iris_Diameter_out_of_range";

	case TemplatePipelineError::Pupil_Iris_ratio_out_of_range:
		return "Pupil_Iris_ratio_out_of_range";

	case TemplatePipelineError::Gaze_out_of_range:
		return "Gaze_out_of_range";

	case TemplatePipelineError::Median_Iris_Intensity_out_of_range:
		return "Median_Iris_Intensity_out_of_range";

	case TemplatePipelineError::Percentage_Of_Iris_Visible_too_small:
		return "Percentage_Of_Iris_Visible_too_small";

	default:
		return "(unrecognized error)";
	}
}

const IrisErrCategory IrisErrCategory { };

std::error_code make_error_code(TemplatePipelineError e) {
	return {static_cast<int>(e), IrisErrCategory};
}

