## Csound based plugin for FMOD Studio. 

More information can be found [here](http://csound.github.io/site/news/2016/07/15/fmod_and_csound)

This repo contains two source files. fmod_csound.cpp is the file that should be used if you wish to build the plugin interface yourself.
The second file, fmod_csound_multi_plug.cpp does not work, but may do in the future depending on developments in the FMOD Plugin API. It's included here as an example of how to create multiple plugin definitions in a single FMOD file. The easiest way to use this framework is with [Cabbage](cabbageaudio.com). However, details are given below on how to use it without having to install [Cabbage](cabbageaudio.com). Note that if you don't use Cabbage, you will need to install the latest version of Csound. 

### Using CsoundFMOD without Cabbage. 

CsoundFMOD is a single plugin library that when loaded by FMOD Studio reads the current directory for a Csound file of the same name. You mst set FMOD Studio to search in the correct folder for your plugins. In order to create new plugins, simply copy and rename the CsoundFMOD library (fmod_csoundL64.dll/fmod_csound.dylib) so that it shares the same name as the .csd file. For example, if you write an instrument called FunkyTown.csd. Copy the fmod_csound library and rename it to FunckyTown.dll, or FunkyTown.dylib. The next time FMOD is started it will load your plugin. 



CsoundFMOD is copyright (c) 2016 Rory Walsh.

CsoundFMOD is free software; you can redistribute them and/or modify them under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

CsoundFMOD is distributed in the hope that they will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this software; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA