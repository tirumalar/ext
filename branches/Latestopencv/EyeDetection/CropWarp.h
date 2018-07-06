#pragma once
#include "pyramid.h"
#include "EyeCenterPoint.h"
void CropAndWarp(const Image8u &src, int x, int y, double zoom, Image8u &dest, void *scratch=0);
void CropAndWarp(const Image8u &src, const CEyeCenterPoint &point, double zoom, Image8u &dest, void *scratch=0);
void CropAndWarp(Pyramid8u &pyramid, int x, int y, float scale, Image8u &output, void *scratch=0);
