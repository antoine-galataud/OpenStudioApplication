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

#include "RefrigerationController.hpp"
#include "RefrigerationGraphicsItems.hpp"
#include "OSAppBase.hpp"
#include "OSDocument.hpp"
#include "OSItem.hpp"
#include "MainWindow.hpp"
#include "MainRightColumnController.hpp"
#include "IconLibrary.hpp"
#include <openstudio/model/Model.hpp>
#include <openstudio/model/Model_Impl.hpp>
#include <openstudio/model/RefrigerationSystem.hpp>
#include <openstudio/model/RefrigerationSystem_Impl.hpp>
#include <openstudio/model/RefrigerationCondenserAirCooled.hpp>
#include <openstudio/model/RefrigerationCondenserAirCooled_Impl.hpp>
#include <openstudio/model/RefrigerationCondenserCascade.hpp>
#include <openstudio/model/RefrigerationCondenserCascade_Impl.hpp>
#include <openstudio/model/RefrigerationCondenserEvaporativeCooled.hpp>
#include <openstudio/model/RefrigerationCondenserEvaporativeCooled_Impl.hpp>
#include <openstudio/model/RefrigerationCondenserWaterCooled.hpp>
#include <openstudio/model/RefrigerationCondenserWaterCooled_Impl.hpp>
#include <openstudio/model/RefrigerationCompressor.hpp>
#include <openstudio/model/RefrigerationCompressor_Impl.hpp>
#include <openstudio/model/RefrigerationCase.hpp>
#include <openstudio/model/RefrigerationCase_Impl.hpp>
#include <openstudio/model/RefrigerationWalkIn.hpp>
#include <openstudio/model/RefrigerationWalkIn_Impl.hpp>
#include <openstudio/model/RefrigerationSubcoolerMechanical.hpp>
#include <openstudio/model/RefrigerationSubcoolerMechanical_Impl.hpp>
#include <openstudio/model/RefrigerationSubcoolerLiquidSuction.hpp>
#include <openstudio/model/RefrigerationSubcoolerLiquidSuction_Impl.hpp>

#include "../model_editor/Utilities.hpp"

#include <openstudio/utilities/core/Compare.hpp>

#include "../shared_gui_components/GraphicsItems.hpp"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QMessageBox>

namespace openstudio {

void RefrigerationController::refreshRefrigerationSystemView(RefrigerationSystemView* systemView,
                                                             boost::optional<model::RefrigerationSystem>& system) {
  OS_ASSERT(systemView);

  systemView->refrigerationCondenserView->setCondenserId(OSItemId());
  systemView->refrigerationSubCoolerView->setId(OSItemId());
  systemView->refrigerationSHXView->setId(OSItemId());
  systemView->refrigerationCasesView->removeAllCaseDetailViews();
  systemView->refrigerationCompressorView->removeAllCompressorDetailViews();
  systemView->refrigerationSecondaryView->removeAllSecondaryDetailViews();

  if (system) {
    systemView->setId(OSItemId(toQString(system->handle()), QString(), false));

    if (boost::optional<model::RefrigerationSubcoolerLiquidSuction> subcooler = system->liquidSuctionHeatExchangerSubcooler()) {
      systemView->refrigerationSHXView->setId(OSItemId(toQString(subcooler->handle()), QString(), false));

      systemView->refrigerationSHXView->setName(QString::fromStdString(subcooler->nameString()));
    }

    if (boost::optional<model::RefrigerationSubcoolerMechanical> subcooler = system->mechanicalSubcooler()) {
      systemView->refrigerationSubCoolerView->setId(OSItemId(toQString(subcooler->handle()), QString(), false));

      systemView->refrigerationSubCoolerView->setName(QString::fromStdString(subcooler->nameString()));
    }

    if (boost::optional<model::ModelObject> condenser = system->refrigerationCondenser()) {
      systemView->refrigerationCondenserView->setCondenserId(OSItemId(toQString(condenser->handle()), QString(), false));

      systemView->refrigerationCondenserView->setCondenserName(QString::fromStdString(condenser->nameString()));

      const QPixmap* pixmap = IconLibrary::Instance().findIcon(condenser->iddObjectType().value());
      systemView->refrigerationCondenserView->setIcon(*pixmap);
    }

    // insert secondary systems

    std::vector<model::RefrigerationCondenserCascade> cascadeCondensers = system->cascadeCondenserLoads();
    for (auto& cascadeCondenser : cascadeCondensers) {
      auto* detailView = new SecondaryDetailView();
      connect(detailView, &SecondaryDetailView::zoomInOnSystemClicked, this,
              static_cast<void (RefrigerationController::*)(const Handle&)>(&RefrigerationController::zoomInOnSystem));
      connect(detailView, &SecondaryDetailView::removeClicked, this, &RefrigerationController::removeLoad);
      if (boost::optional<model::RefrigerationSystem> t_cascadeSystem = cascadeSystem(cascadeCondenser)) {
        detailView->setName(QString::fromStdString(t_cascadeSystem->nameString()));
        detailView->setHandle(t_cascadeSystem->handle());
      } else {
        detailView->setName(QString::fromStdString(cascadeCondenser.nameString()));
        detailView->setHandle(cascadeCondenser.handle());
      }
      systemView->refrigerationSecondaryView->insertSecondaryDetailView(0, detailView);
    }

    // insert compressors

    std::vector<model::RefrigerationCompressor> compressors = system->compressors();

    int compressorIndex = 1;

    for (auto& compressor : compressors) {
      auto* detailView = new RefrigerationCompressorDetailView();

      detailView->setId(OSItemId(toQString(compressor.handle()), QString(), false));

      detailView->setLabel(QString::number(compressorIndex));

      connect(detailView, &RefrigerationCompressorDetailView::removeClicked, this, &RefrigerationController::removeCompressor);

      connect(detailView, &RefrigerationCompressorDetailView::inspectClicked, this, &RefrigerationController::inspectOSItem);

      systemView->refrigerationCompressorView->insertCompressorDetailView(0, detailView);

      compressorIndex++;
    }

    // insert cases

    std::vector<model::RefrigerationCase> cases = system->cases();

    systemView->refrigerationCasesView->setNumberOfDisplayCases(cases.size());

    for (auto& c : cases) {
      auto* detailView = new RefrigerationCaseDetailView();

      detailView->setId(OSItemId(toQString(c.handle()), QString(), false));

      detailView->setName(QString::fromStdString(c.nameString()));

      connect(detailView, &RefrigerationCaseDetailView::removeClicked, this, &RefrigerationController::removeCase);

      connect(detailView, &RefrigerationCaseDetailView::inspectClicked, this, &RefrigerationController::inspectOSItem);

      systemView->refrigerationCasesView->insertCaseDetailView(0, detailView);
    }

    // insert walkins

    std::vector<model::RefrigerationWalkIn> walkins = system->walkins();

    systemView->refrigerationCasesView->setNumberOfWalkinCases(walkins.size());

    for (auto& walkin : walkins) {
      auto* detailView = new RefrigerationCaseDetailView();

      detailView->setId(OSItemId(toQString(walkin.handle()), QString(), false));

      detailView->setName(QString::fromStdString(walkin.nameString()));

      connect(detailView, &RefrigerationCaseDetailView::removeClicked, this, &RefrigerationController::removeCase);

      connect(detailView, &RefrigerationCaseDetailView::inspectClicked, this, &RefrigerationController::inspectOSItem);

      systemView->refrigerationCasesView->insertCaseDetailView(0, detailView);
    }

    systemView->adjustLayout();
  }
}

RefrigerationController::RefrigerationController()
  : m_refrigerationView(new RefrigerationView()),
    m_refrigerationSystemGridView(new GridLayoutItem()),
    m_refrigerationGridScene(new QGraphicsScene()),
    m_noRefrigerationView(new NoRefrigerationView()),
    m_dirty(false) {
  connect(m_refrigerationView->zoomOutButton, &QPushButton::clicked, this, &RefrigerationController::zoomOutToSystemGridView);

  // These get deleted with when the scene is deleted
  m_refrigerationSystemGridView->setCellSize(RefrigerationSystemMiniView::cellSize());
  m_refrigerationSystemGridView->setMargin(RefrigerationSystemView::margin);

  m_refrigerationSystemListController = QSharedPointer<RefrigerationSystemListController>(new RefrigerationSystemListController(this));
  m_refrigerationSystemGridView->setListController(m_refrigerationSystemListController);

  m_refrigerationSystemGridView->setDelegate(QSharedPointer<RefrigerationSystemItemDelegate>(new RefrigerationSystemItemDelegate()));

  m_refrigerationGridScene->addItem(m_refrigerationSystemGridView);

  zoomOutToSystemGridView();
}

RefrigerationController::~RefrigerationController() {
  delete m_refrigerationView;

  delete m_refrigerationScene;

  delete m_noRefrigerationView;
}

boost::optional<model::RefrigerationSystem> RefrigerationController::supplySystem(const model::RefrigerationCondenserCascade& condenser) {
  boost::optional<model::RefrigerationSystem> result;

  model::Model t_model = condenser.model();
  std::vector<model::RefrigerationSystem> systems = t_model.getConcreteModelObjects<model::RefrigerationSystem>();
  for (auto& system : systems) {
    std::vector<model::RefrigerationCondenserCascade> condenserLoads = system.cascadeCondenserLoads();
    if (std::find(condenserLoads.begin(), condenserLoads.end(), condenser) != condenserLoads.end()) {
      result = system;
      break;
    }
  }

  return result;
}

boost::optional<model::RefrigerationSystem> RefrigerationController::cascadeSystem(const model::RefrigerationCondenserCascade& condenser) {
  boost::optional<model::RefrigerationSystem> result;

  model::Model t_model = condenser.model();
  std::vector<model::RefrigerationSystem> systems = t_model.getConcreteModelObjects<model::RefrigerationSystem>();
  for (auto& system : systems) {
    if (boost::optional<model::ModelObject> mo = system.refrigerationCondenser()) {
      if (mo->handle() == condenser.handle()) {
        result = system;
        break;
      }
    }
  }

  return result;
}

void RefrigerationController::zoomInOnSystem(const Handle& handle) {
  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();
  model::Model t_model = doc->model();

  if (boost::optional<model::RefrigerationSystem> system = t_model.getModelObject<model::RefrigerationSystem>(handle)) {
    zoomInOnSystem(system.get());
  }
}

void RefrigerationController::zoomInOnSystem(const model::RefrigerationSystem& refrigerationSystem) {
  model::OptionalModelObject mo;

  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  doc->mainRightColumnController()->inspectModelObject(mo, false);

  m_currentSystem = refrigerationSystem;

  if (m_refrigerationScene) {
    m_refrigerationScene->deleteLater();
  }

  m_refrigerationScene = new QGraphicsScene();

  m_refrigerationView->header->show();

  m_detailView = new RefrigerationSystemView();

  refresh();

  m_refrigerationScene->addItem(m_detailView);

  m_refrigerationView->graphicsView->setScene(m_refrigerationScene);

  connect(m_detailView->refrigerationCondenserView, &RefrigerationCondenserView::componentDropped, this,
          &RefrigerationController::onCondenserViewDrop);

  connect(m_detailView->refrigerationCondenserView, &RefrigerationCondenserView::removeClicked, this, &RefrigerationController::removeCondenser);

  connect(m_detailView->refrigerationCondenserView, &RefrigerationCondenserView::inspectClicked, this, &RefrigerationController::inspectOSItem);

  connect(m_detailView->refrigerationCompressorView->refrigerationCompressorDropZoneView, &RefrigerationCompressorDropZoneView::componentDropped,
          this, &RefrigerationController::onCompressorViewDrop);

  connect(m_detailView->refrigerationCasesView->refrigerationCasesDropZoneView, &RefrigerationCasesDropZoneView::componentDropped, this,
          &RefrigerationController::onCasesViewDrop);

  connect(m_detailView->refrigerationSubCoolerView, &RefrigerationSubCoolerView::componentDropped, this,
          &RefrigerationController::onSubCoolerViewDrop);

  connect(m_detailView->refrigerationSubCoolerView, &RefrigerationSubCoolerView::removeClicked, this, &RefrigerationController::removeSubCooler);

  connect(m_detailView->refrigerationSubCoolerView, &RefrigerationSubCoolerView::inspectClicked, this, &RefrigerationController::inspectOSItem);

  connect(m_detailView->refrigerationSHXView, &RefrigerationSHXView::componentDropped, this, &RefrigerationController::onSHXViewDrop);

  connect(m_detailView->refrigerationSHXView, &RefrigerationSHXView::removeClicked, this, &RefrigerationController::removeSubCoolerLiquidSuction);

  connect(m_detailView->refrigerationSHXView, &RefrigerationSHXView::inspectClicked, this, &RefrigerationController::inspectOSItem);

  connect(m_detailView.data(), &RefrigerationSystemView::inspectClicked, this, &RefrigerationController::inspectOSItem);

  connect(m_detailView->refrigerationSecondaryView->secondaryDropZoneView, &SecondaryDropZoneView::componentDropped, this,
          &RefrigerationController::onSecondaryViewDrop);

  m_refrigerationView->graphicsView->setAlignment(Qt::AlignCenter);
}

void RefrigerationController::zoomOutToSystemGridView() {
  model::OptionalModelObject mo;

  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  doc->mainRightColumnController()->inspectModelObject(mo, false);

  m_currentSystem = boost::none;

  m_refrigerationView->header->hide();

  m_refrigerationSystemListController->reset();

  refresh();

  m_refrigerationView->graphicsView->setScene(m_refrigerationGridScene.data());

  m_refrigerationView->graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void RefrigerationController::onCondenserViewDrop(const OSItemId& itemid) {
  OS_ASSERT(m_currentSystem);

  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  if (doc->fromComponentLibrary(itemid)) {
    boost::optional<model::ModelObject> mo = doc->getModelObject(itemid);

    OS_ASSERT(mo);

    if (boost::optional<model::RefrigerationCondenserAirCooled> condenser = mo->optionalCast<model::RefrigerationCondenserAirCooled>()) {
      if (boost::optional<model::ModelObject> currentCondenser = m_currentSystem->refrigerationCondenser()) {
        currentCondenser->remove();
      }

      auto condenserClone = condenser->clone(m_currentSystem->model()).cast<model::RefrigerationCondenserAirCooled>();

      m_currentSystem->setRefrigerationCondenser(condenserClone);

      refresh();
    } else if (boost::optional<model::RefrigerationCondenserCascade> condenser = mo->optionalCast<model::RefrigerationCondenserCascade>()) {
      if (boost::optional<model::ModelObject> currentCondenser = m_currentSystem->refrigerationCondenser()) {
        currentCondenser->remove();
      }

      auto condenserClone = condenser->clone(m_currentSystem->model()).cast<model::RefrigerationCondenserCascade>();

      m_currentSystem->setRefrigerationCondenser(condenserClone);

      refresh();
    } else if (boost::optional<model::RefrigerationCondenserEvaporativeCooled> condenser =
                 mo->optionalCast<model::RefrigerationCondenserEvaporativeCooled>()) {
      if (boost::optional<model::ModelObject> currentCondenser = m_currentSystem->refrigerationCondenser()) {
        currentCondenser->remove();
      }

      auto condenserClone = condenser->clone(m_currentSystem->model()).cast<model::RefrigerationCondenserEvaporativeCooled>();

      m_currentSystem->setRefrigerationCondenser(condenserClone);

      refresh();
    } else if (boost::optional<model::RefrigerationCondenserWaterCooled> condenser = mo->optionalCast<model::RefrigerationCondenserWaterCooled>()) {
      if (boost::optional<model::ModelObject> currentCondenser = m_currentSystem->refrigerationCondenser()) {
        currentCondenser->remove();
      }

      auto condenserClone = condenser->clone(m_currentSystem->model()).cast<model::RefrigerationCondenserWaterCooled>();

      m_currentSystem->setRefrigerationCondenser(condenserClone);

      refresh();
    }
  }
}

void RefrigerationController::onCompressorViewDrop(const OSItemId& itemid) {
  OS_ASSERT(m_currentSystem);

  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  if (doc->fromComponentLibrary(itemid)) {
    boost::optional<model::ModelObject> mo = doc->getModelObject(itemid);

    OS_ASSERT(mo);

    if (boost::optional<model::RefrigerationCompressor> compressor = mo->optionalCast<model::RefrigerationCompressor>()) {
      auto compressorClone = compressor->clone(m_currentSystem->model()).cast<model::RefrigerationCompressor>();

      m_currentSystem->addCompressor(compressorClone);

      refresh();
    }
  }
}

void RefrigerationController::onSecondaryViewDrop(const OSItemId& itemid) {
  OS_ASSERT(m_currentSystem);
  model::Model t_model = m_currentSystem->model();

  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();
  if (doc->fromComponentLibrary(itemid)) {
    boost::optional<model::ModelObject> mo = doc->getModelObject(itemid);
    OS_ASSERT(mo);

    if (boost::optional<model::RefrigerationSystem> system = mo->optionalCast<model::RefrigerationSystem>()) {
      if (boost::optional<model::ModelObject> condenserModelObject = system->refrigerationCondenser()) {
        if (boost::optional<model::RefrigerationCondenserCascade> cascadeCondenser =
              condenserModelObject->optionalCast<model::RefrigerationCondenserCascade>()) {
          // If condenser is not already a load on another system
          if (!supplySystem(cascadeCondenser.get())) {
            auto systemClone = system->clone(t_model).cast<model::RefrigerationSystem>();
            boost::optional<model::RefrigerationCondenserCascade> cascadeCondenserClone =
              systemClone.refrigerationCondenser()->optionalCast<model::RefrigerationCondenserCascade>();
            OS_ASSERT(cascadeCondenserClone);
            m_currentSystem->addCascadeCondenserLoad(cascadeCondenserClone.get());

            refresh();
          }
        }
      } else {
        auto systemClone = system->clone(t_model).cast<model::RefrigerationSystem>();

        model::RefrigerationCondenserCascade newCascadeCondenser(t_model);
        systemClone.setRefrigerationCondenser(newCascadeCondenser);
        m_currentSystem->addCascadeCondenserLoad(newCascadeCondenser);

        refresh();
      }
    }
  } else if (doc->fromModel(itemid)) {
    boost::optional<model::ModelObject> mo = doc->getModelObject(itemid);
    OS_ASSERT(mo);

    if (boost::optional<model::RefrigerationSystem> system = mo->optionalCast<model::RefrigerationSystem>()) {
      if (boost::optional<model::ModelObject> condenserModelObject = system->refrigerationCondenser()) {
        if (boost::optional<model::RefrigerationCondenserCascade> cascadeCondenser =
              condenserModelObject->optionalCast<model::RefrigerationCondenserCascade>()) {
          // If condenser is not already a load on another system
          if (!supplySystem(cascadeCondenser.get())) {
            m_currentSystem->addCascadeCondenserLoad(cascadeCondenser.get());
            refresh();
          }
        }
      } else {
        model::RefrigerationCondenserCascade newCascadeCondenser(t_model);
        system->setRefrigerationCondenser(newCascadeCondenser);
        m_currentSystem->addCascadeCondenserLoad(newCascadeCondenser);
        refresh();
      }
    }
  }
}

void RefrigerationController::onCasesViewDrop(const OSItemId& itemid) {
  OS_ASSERT(m_currentSystem);

  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  if (doc->fromComponentLibrary(itemid)) {
    boost::optional<model::ModelObject> mo = doc->getModelObject(itemid);

    OS_ASSERT(mo);

    if (boost::optional<model::RefrigerationCase> _case = mo->optionalCast<model::RefrigerationCase>()) {
      auto caseClone = _case->clone(m_currentSystem->model()).cast<model::RefrigerationCase>();

      m_currentSystem->addCase(caseClone);

      refresh();
    } else if (boost::optional<model::RefrigerationWalkIn> _walkin = mo->optionalCast<model::RefrigerationWalkIn>()) {
      auto walkinClone = _walkin->clone(m_currentSystem->model()).cast<model::RefrigerationWalkIn>();
      m_currentSystem->addWalkin(walkinClone);
      refresh();
      //QMessageBox box(doc->mainWindow());
      //box.setText("Walkins are not yet supported by OpenStudio.");
      //box.exec();
    }
  }
}

void RefrigerationController::onSubCoolerViewDrop(const OSItemId& itemid) {
  OS_ASSERT(m_currentSystem);

  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  if (doc->fromComponentLibrary(itemid)) {
    boost::optional<model::ModelObject> mo = doc->getModelObject(itemid);

    OS_ASSERT(mo);

    if (boost::optional<model::RefrigerationSubcoolerMechanical> _subcooler = mo->optionalCast<model::RefrigerationSubcoolerMechanical>()) {
      auto subcoolerClone = _subcooler->clone(m_currentSystem->model()).cast<model::RefrigerationSubcoolerMechanical>();

      m_currentSystem->setMechanicalSubcooler(subcoolerClone);

      refresh();
    }
  }
}

void RefrigerationController::removeSubCooler(const OSItemId& itemid) {
  if (boost::optional<model::Model> model = OSAppBase::instance()->currentModel()) {
    if (boost::optional<model::ModelObject> mo = model->getModelObject<model::ModelObject>(toUUID(itemid.itemId()))) {
      mo->remove();

      refresh();
    }
  }
}

void RefrigerationController::removeLoad(const Handle& handle) {
  if (boost::optional<model::Model> model = OSAppBase::instance()->currentModel()) {
    if (boost::optional<model::RefrigerationSystem> cascadeSystem_ = model->getModelObject<model::RefrigerationSystem>(handle)) {
      OS_ASSERT(m_currentSystem);
      if (boost::optional<model::ModelObject> condenserModelObject = cascadeSystem_->refrigerationCondenser()) {
        if (boost::optional<model::RefrigerationCondenserCascade> condenserCascade =
              condenserModelObject->optionalCast<model::RefrigerationCondenserCascade>()) {
          m_currentSystem->removeCascadeCondenserLoad(condenserCascade.get());
          refresh();
        }
      }
    } else if (boost::optional<model::RefrigerationCondenserCascade> condenserCascade =
                 model->getModelObject<model::RefrigerationCondenserCascade>(handle)) {
      condenserCascade->remove();
      refresh();
    }
  }
}

void RefrigerationController::onSHXViewDrop(const OSItemId& itemid) {
  OS_ASSERT(m_currentSystem);

  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  if (doc->fromComponentLibrary(itemid)) {
    boost::optional<model::ModelObject> mo = doc->getModelObject(itemid);

    OS_ASSERT(mo);

    if (boost::optional<model::RefrigerationSubcoolerLiquidSuction> _subcooler = mo->optionalCast<model::RefrigerationSubcoolerLiquidSuction>()) {
      auto subcoolerClone = _subcooler->clone(m_currentSystem->model()).cast<model::RefrigerationSubcoolerLiquidSuction>();

      m_currentSystem->setLiquidSuctionHeatExchangerSubcooler(subcoolerClone);

      refresh();
    }
  }
}

void RefrigerationController::removeSubCoolerLiquidSuction(const OSItemId& itemid) {
  if (boost::optional<model::Model> model = OSAppBase::instance()->currentModel()) {
    if (boost::optional<model::ModelObject> mo = model->getModelObject<model::ModelObject>(toUUID(itemid.itemId()))) {
      mo->remove();

      refresh();
    }
  }
}

void RefrigerationController::refresh() {
  m_dirty = true;

  QTimer::singleShot(0, this, &RefrigerationController::refreshNow);
}

void RefrigerationController::refreshNow() {
  if (!m_dirty) {
    return;
  }

  if (m_detailView) {
    refreshRefrigerationSystemView(m_detailView, m_currentSystem);
  }

  m_dirty = false;
}

RefrigerationView* RefrigerationController::refrigerationView() const {
  return m_refrigerationView;
}

NoRefrigerationView* RefrigerationController::noRefrigerationView() const {
  return m_noRefrigerationView;
}

void RefrigerationController::inspectOSItem(const OSItemId& itemid) {
  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  OS_ASSERT(doc);

  OS_ASSERT(m_currentSystem);

  boost::optional<model::ModelObject> mo = m_currentSystem->model().getModelObject<model::ModelObject>(toUUID(itemid.itemId()));

  doc->mainRightColumnController()->inspectModelObject(mo, false);
}

void RefrigerationController::removeCompressor(const OSItemId& itemid) {
  if (boost::optional<model::Model> model = OSAppBase::instance()->currentModel()) {
    if (boost::optional<model::ModelObject> mo = model->getModelObject<model::ModelObject>(toUUID(itemid.itemId()))) {
      mo->remove();

      refresh();
    }
  }
}

void RefrigerationController::removeCondenser(const OSItemId& itemid) {
  if (boost::optional<model::Model> model = OSAppBase::instance()->currentModel()) {
    if (boost::optional<model::ModelObject> mo = model->getModelObject<model::ModelObject>(toUUID(itemid.itemId()))) {
      mo->remove();

      OS_ASSERT(m_detailView);

      m_detailView->refrigerationCondenserView->setCondenserId(OSItemId());
    }
  }
}

void RefrigerationController::removeCase(const OSItemId& itemid) {
  if (boost::optional<model::Model> model = OSAppBase::instance()->currentModel()) {
    if (boost::optional<model::ModelObject> mo = model->getModelObject<model::ModelObject>(toUUID(itemid.itemId()))) {
      mo->remove();

      refresh();
    }
  }
}

QSharedPointer<RefrigerationSystemListController> RefrigerationController::refrigerationSystemListController() const {
  return m_refrigerationSystemListController;
}

RefrigerationSystemListController::RefrigerationSystemListController(RefrigerationController* refrigerationController)
  : m_refrigerationController(refrigerationController) {
  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();
  model::Model t_model = doc->model();
  t_model.getImpl<model::detail::Model_Impl>()
    ->addWorkspaceObject.connect<RefrigerationSystemListController, &RefrigerationSystemListController::onModelObjectAdd>(this);

  connect(this, &RefrigerationSystemListController::itemInsertedPrivate, this, &RefrigerationSystemListController::itemInserted,
          Qt::QueuedConnection);
}

void RefrigerationSystemListController::reset() {
  emit modelReset();
}

RefrigerationController* RefrigerationSystemListController::refrigerationController() const {
  return m_refrigerationController;
}

QSharedPointer<OSListItem> RefrigerationSystemListController::itemAt(int i) {
  QSharedPointer<OSListItem> item;

  if (i == 0) {
    item = QSharedPointer<RefrigerationSystemListDropZoneItem>(new RefrigerationSystemListDropZoneItem(this));
  } else if (i > 0 && i < count()) {
    item = QSharedPointer<RefrigerationSystemListItem>(new RefrigerationSystemListItem(systems()[i - 1], this));
  }

  return item;
}

int RefrigerationSystemListController::count() {
  return systems().size() + 1;
}

int RefrigerationSystemListController::systemIndex(const model::RefrigerationSystem& system) const {

  int i = 1;

  for (const auto& thisSystem : systems()) {
    if (system == thisSystem) {
      break;
    }

    i++;
  }

  return i;
}

void RefrigerationSystemListController::createNewSystem() {
  if (boost::optional<model::Model> model = OSAppBase::instance()->currentModel()) {
    model::RefrigerationSystem system(model.get());
  }
}

void RefrigerationSystemListController::onModelObjectAdd(const WorkspaceObject& object, const openstudio::IddObjectType& iddObjectType,
                                                         const openstudio::UUID& /*handle*/) {
  if (iddObjectType == model::RefrigerationSystem::iddObjectType()) {
    emit itemInsertedPrivate(systemIndex(object.cast<model::RefrigerationSystem>()));
  }
}

void RefrigerationSystemListController::addSystem(const OSItemId& itemid) {
  std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  if (doc->fromComponentLibrary(itemid)) {
    boost::optional<model::ModelObject> mo = doc->getModelObject(itemid);

    boost::optional<model::Model> model = OSAppBase::instance()->currentModel();

    if (mo && model) {
      if (boost::optional<model::RefrigerationSystem> system = mo->optionalCast<model::RefrigerationSystem>()) {
        // model::RefrigerationSystem systemClone = system->clone(model.get()).cast<model::RefrigerationSystem>();
        system->clone(model.get());
      }
    }
  }
}

void RefrigerationSystemListController::removeSystem(model::RefrigerationSystem& refrigerationSystem) {
  int i = systemIndex(refrigerationSystem);

  refrigerationSystem.remove();

  emit itemRemoved(i);
}

std::vector<model::RefrigerationSystem> RefrigerationSystemListController::systems() const {
  std::vector<model::RefrigerationSystem> result;

  if (boost::optional<model::Model> model = OSAppBase::instance()->currentModel()) {
    result = model->getConcreteModelObjects<model::RefrigerationSystem>();
  }

  std::sort(result.begin(), result.end(), WorkspaceObjectNameLess());

  return result;
}

RefrigerationSystemListItem::RefrigerationSystemListItem(const model::RefrigerationSystem& refrigerationSystem, OSListController* listController)
  : OSListItem(listController), m_refrigerationSystem(refrigerationSystem) {}

QString RefrigerationSystemListItem::systemName() const {
  return QString::fromStdString(m_refrigerationSystem.nameString());
}

model::RefrigerationSystem RefrigerationSystemListItem::system() const {
  return m_refrigerationSystem;
}

void RefrigerationSystemListItem::remove() {
  qobject_cast<RefrigerationSystemListController*>(controller())->removeSystem(m_refrigerationSystem);
}

void RefrigerationSystemListItem::zoomInOnSystem() {
  qobject_cast<RefrigerationSystemListController*>(controller())->refrigerationController()->zoomInOnSystem(m_refrigerationSystem);
}

RefrigerationSystemListDropZoneItem::RefrigerationSystemListDropZoneItem(OSListController* listController) : OSListItem(listController) {}

QGraphicsObject* RefrigerationSystemItemDelegate::view(QSharedPointer<OSListItem> dataSource) {
  QGraphicsObject* itemView = nullptr;

  if (QSharedPointer<RefrigerationSystemListItem> listItem = dataSource.dynamicCast<RefrigerationSystemListItem>()) {
    auto* refrigerationSystemMiniView = new RefrigerationSystemMiniView();

    connect(refrigerationSystemMiniView->removeButtonItem, &RemoveButtonItem::mouseClicked,
            static_cast<RefrigerationSystemListItem*>(dataSource.data()), &RefrigerationSystemListItem::remove);

    connect(refrigerationSystemMiniView->zoomInButtonItem, &ZoomInButtonItem::mouseClicked,
            static_cast<RefrigerationSystemListItem*>(dataSource.data()), &RefrigerationSystemListItem::zoomInOnSystem);

    refrigerationSystemMiniView->setName(listItem->systemName());

    boost::optional<model::RefrigerationSystem> system = listItem->system();
    RefrigerationController* refrigerationController =
      qobject_cast<RefrigerationSystemListController*>(dataSource->controller())->refrigerationController();
    refrigerationController->refreshRefrigerationSystemView(refrigerationSystemMiniView->refrigerationSystemView, system);
    refrigerationSystemMiniView->adjustLayout();

    itemView = refrigerationSystemMiniView;
  } else if (dataSource.dynamicCast<RefrigerationSystemListDropZoneItem>()) {
    auto* refrigerationSystemDropZoneView = new RefrigerationSystemDropZoneView();

    connect(refrigerationSystemDropZoneView, &RefrigerationSystemDropZoneView::componentDropped,
            qobject_cast<RefrigerationSystemListController*>(dataSource->controller()), &RefrigerationSystemListController::addSystem);

    itemView = refrigerationSystemDropZoneView;
  }

  return itemView;
}

}  // namespace openstudio
