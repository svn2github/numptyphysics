/*
 * This file is part of NumptyPhysics
 * Copyright (C) 2009 Tim Edmonds
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */
#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Common.h>

class Accelerometer
{
 public:  
  // in Gs
  virtual bool poll( float32& gx, float32& gy, float32& gz )=0;
};


#endif //ACCELEROMETER_H
