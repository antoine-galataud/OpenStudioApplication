/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) 2020-2022, OpenStudio Coalition and other contributors. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
*  disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote products
*  derived from this software without specific prior written permission from the respective party.
*
*  (4) Other than as required in clauses (1) and (2), distributions in any form of modifications or other derivative works
*  may not use the "OpenStudio" trademark, "OS", "os", or any other confusingly similar designation without specific prior
*  written permission from Alliance for Sustainable Energy, LLC.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND ANY CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S), ANY CONTRIBUTORS, THE UNITED STATES GOVERNMENT, OR THE UNITED
*  STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
*  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************************************************************/

#ifndef OPENSTUDIO_RUNTABCONTROLLER_HPP
#define OPENSTUDIO_RUNTABCONTROLLER_HPP

#include "MainTabController.hpp"
#include <openstudio/utilities/core/Path.hpp>

namespace openstudio {

//namespace runmanager {
//  class RunManager;
//  class JobStatusWidget;
//}

namespace model {
class Model;
}

class RunView;

class RunTabController : public MainTabController
{
  Q_OBJECT

 public:
  RunTabController(const model::Model& model, const openstudio::path& t_modelPath,
                   const openstudio::path& t_tempFolder);  //, openstudio::runmanager::RunManager t_runManager);

  virtual ~RunTabController() {}

  //openstudio::RunView * runView();

  //runmanager::RunManager runManager();

  enum TabID
  {
    OUTPUT,
    TREE
  };

 signals:
  void resultsGenerated(const openstudio::path& t_sqlFile, const openstudio::path& t_radianceOutputFile);
  void toolsUpdated();
  //void useRadianceStateChanged(bool);

 public slots:
  void updateToolsWarnings();

 private:
  //RunView * m_runView;

  //openstudio::runmanager::JobStatusWidget * m_status;
};

}  // namespace openstudio

#endif  // OPENSTUDIO_RUNTABCONTROLLER_HPP
