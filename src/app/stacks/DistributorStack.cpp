//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================

#include "DistributorStack.h"
#include <plugins/plugin_distribute.h>
#include <utility>

DistributorStack::DistributorStack(RenderOptions *_opts, FrameBuffer *_fb,
                                   vector<CaptureSplitter *> captureSplitters)
    : VisionStack(_opts) {
  stack.push_back(new PluginDistribute(_fb, std::move(captureSplitters)));
}