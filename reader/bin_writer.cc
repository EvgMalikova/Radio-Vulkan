 /*
 * Copyright (c) 2004-2014
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <iostream>
#include <vector>
#include <string>

#include "cxxsupport/arr.h"
#include "cxxsupport/paramfile.h"
#include "cxxsupport/mpi_support.h"
#include "cxxsupport/bstream.h"
#include "splotch/splotchutils.h"

using namespace std;

  void write_binary(const char*filename,vector<particle_sim> particle_data){
    bofstream wf(filename,false);
    for(int i = 0; i < particle_data.size(); i++)
    {
         wf.write((char *) &particle_data[i].x, sizeof(float32));
         wf.write((char *) &particle_data[i].y, sizeof(float32));
         wf.write((char *) &particle_data[i].z, sizeof(float32));
         wf.write((char *) &particle_data[i].r, sizeof(float32));
         wf.write((char *) &particle_data[i].I, sizeof(float32));
         wf.write((char *) &particle_data[i].e.r, sizeof(float));
         wf.write((char *) &particle_data[i].e.g, sizeof(float));
         wf.write((char *) &particle_data[i].e.b, sizeof(float));
         
    }
    wf.close();
  }

