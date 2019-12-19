/*
 * TemplatePipelineError.h
 * Anita
 */
#pragma once

#include <stdio.h>
#include <iostream>
#include <system_error>


enum class TemplatePipelineError {
	Segmentation_Successful = 0,
	Pupil_Iris_Boundary_Not_Found = 100,
	Iris_Sclera_Boundary_Not_Found = 101,
	Usable_Iris_Area_not_sufficient = 102,
	Pupil_Diameter_out_of_range = 103,
	Iris_Diameter_out_of_range = 104, 			// reject unless D >= 73
	Pupil_Iris_ratio_out_of_range = 105,		// ignore for authentication mode
	Gaze_out_of_range = 106,
	Median_Iris_Intensity_out_of_range = 107,
	Percentage_Of_Iris_Visible_too_small = 108,
	InValidbits_in_Template = 109, 				// reject if V < 1500
};

namespace std
{
  template<>
    struct is_error_code_enum<TemplatePipelineError> : true_type {};
}

std::error_code make_error_code(TemplatePipelineError);

