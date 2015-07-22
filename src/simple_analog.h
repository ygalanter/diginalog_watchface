#pragma once

#include "pebble.h"


static const GPathInfo MINUTE_HAND_POINTS =
{
  5,
  (GPoint []) {
    {-5, -75},  
    { 0, -80 },      
    { 5, -75 },      
    { 5, 0},      
    { -5, 0 }      
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  5, (GPoint []){
    {-6, -54},
    {0, -60},       
    {6, -54},      
    {6, 0},      
    {-6, 0}      
  }
};
