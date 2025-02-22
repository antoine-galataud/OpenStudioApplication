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

#include "YearSettingsWidget.hpp"

#include "OSAppBase.hpp"
#include "OSDocument.hpp"

#include "../shared_gui_components/OSComboBox.hpp"
#include "../shared_gui_components/OSSwitch.hpp"

#include <openstudio/model/RunPeriodControlDaylightSavingTime.hpp>
#include <openstudio/model/RunPeriodControlDaylightSavingTime_Impl.hpp>
#include <openstudio/model/WeatherFile.hpp>
#include <openstudio/model/WeatherFile_Impl.hpp>

#include "../model_editor/Utilities.hpp"

#include <openstudio/utilities/core/Compare.hpp>
#include <openstudio/utilities/filetypes/EpwFile.hpp>
#include <openstudio/utilities/idd/IddEnums.hxx>
#include <openstudio/utilities/time/Date.hpp>

#include <QBoxLayout>
#include <QButtonGroup>
#include <QDateEdit>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDebug>

namespace openstudio {

const int YearSettingsWidget::FIRSTYEAR = 1900;
const int YearSettingsWidget::LASTYEAR = 2100;

YearSettingsWidget::YearSettingsWidget(const model::Model& model, QWidget* parent)
  : QWidget(parent), m_dstOnOffButton(nullptr), m_model(model), m_dirty(false) {
  // Main Layout

  auto* mainVLayout = new QVBoxLayout();
  mainVLayout->setContentsMargins(10, 10, 10, 10);
  mainVLayout->setSpacing(10);

  setLayout(mainVLayout);

  // YearDescription

  m_yearDescription = m_model.getUniqueModelObject<model::YearDescription>();

  // Year

  auto* selectYearLabel = new QLabel(tr("Select Year by:"));
  selectYearLabel->setObjectName("H2");
  mainVLayout->addWidget(selectYearLabel);

  auto* yearGridLayout = new QGridLayout();
  yearGridLayout->setContentsMargins(20, 10, 10, 0);
  //yearGridLayout->setContentsMargins(20,10,10,0);
  yearGridLayout->setSpacing(10);
  mainVLayout->addLayout(yearGridLayout);

  auto* yearButtonGroup = new QButtonGroup(this);

  m_calendarYearButton = new QRadioButton(tr("Calendar Year"), this);
  yearGridLayout->addWidget(m_calendarYearButton, 0, 0);
  yearButtonGroup->addButton(m_calendarYearButton);
  connect(m_calendarYearButton, &QRadioButton::clicked, this, &YearSettingsWidget::onCalendarYearButtonClicked);
  m_calendarYearEdit = new OSComboBox2();
  for (int i = FIRSTYEAR; i <= LASTYEAR; i++) {
    m_calendarYearEdit->addItem(QString::number(i));
  }
  m_calendarYearEdit->setCurrentIndex(100);
  connect(m_calendarYearEdit, static_cast<void (OSComboBox2::*)(int)>(&OSComboBox2::currentIndexChanged), this,
          &YearSettingsWidget::onCalendarYearChanged);
  yearGridLayout->addWidget(m_calendarYearEdit, 0, 1);

  m_firstDayOfYearButton = new QRadioButton(tr("First Day of Year"), this);
  yearGridLayout->addWidget(m_firstDayOfYearButton, 1, 0);
  yearButtonGroup->addButton(m_firstDayOfYearButton);
  connect(m_firstDayOfYearButton, &QRadioButton::clicked, this, &YearSettingsWidget::onFirstDayofYearClicked);
  m_firstDayOfYearEdit = new OSComboBox2();
  m_firstDayOfYearEdit->addItems(validDayofWeekforStartDay());
  connect(m_firstDayOfYearEdit, static_cast<void (OSComboBox2::*)(const QString&)>(&OSComboBox2::currentTextChanged), this,
          &YearSettingsWidget::firstDayofYearSelected);
  yearGridLayout->addWidget(m_firstDayOfYearEdit, 1, 1);

  yearGridLayout->setColumnStretch(2, 10);

  auto* line1 = new QFrame();
  line1->setFrameShape(QFrame::HLine);
  line1->setFrameShadow(QFrame::Sunken);
  mainVLayout->addWidget(line1);

  // DST

  auto* dstHLayout1 = new QHBoxLayout();
  dstHLayout1->setContentsMargins(0, 0, 0, 0);
  dstHLayout1->setSpacing(10);

  auto* dstLabel = new QLabel(tr("Daylight Savings Time:"));
  dstLabel->setObjectName("H2");
  dstHLayout1->addWidget(dstLabel);

  m_dstOnOffButton = new OSSwitch2();
  dstHLayout1->addWidget(m_dstOnOffButton);
  connect(m_dstOnOffButton, &OSSwitch2::clicked, this, &YearSettingsWidget::daylightSavingTimeClicked);
  dstHLayout1->addStretch();

  mainVLayout->addLayout(dstHLayout1);

  auto* dstGridLayout = new QGridLayout();
  dstGridLayout->setContentsMargins(20, 10, 10, 10);
  dstGridLayout->setSpacing(10);
  mainVLayout->addLayout(dstGridLayout);

  // Starts

  auto* startsLabel = new QLabel(tr("Starts"));
  startsLabel->setObjectName("H2");
  dstGridLayout->addWidget(startsLabel, 0, 0);

  auto* dstStartButtonGroup = new QButtonGroup(this);

  m_dayOfWeekAndMonthStartButton = new QRadioButton(tr("Define by Day of The Week And Month"), this);
  dstStartButtonGroup->addButton(m_dayOfWeekAndMonthStartButton);
  dstGridLayout->addWidget(m_dayOfWeekAndMonthStartButton, 1, 0);
  connect(m_dayOfWeekAndMonthStartButton, &QRadioButton::clicked, this, &YearSettingsWidget::onDefineStartByDayWeekMonthClicked);
  m_startWeekBox = new OSComboBox2();
  dstGridLayout->addWidget(m_startWeekBox, 1, 1);

  m_startDayBox = new OSComboBox2();
  dstGridLayout->addWidget(m_startDayBox, 1, 2);

  m_startMonthBox = new OSComboBox2();
  dstGridLayout->addWidget(m_startMonthBox, 1, 3);

  m_dateStartButton = new QRadioButton(tr("Define by Date"), this);
  dstStartButtonGroup->addButton(m_dateStartButton);
  dstGridLayout->addWidget(m_dateStartButton, 2, 0);
  connect(m_dateStartButton, &QRadioButton::clicked, this, &YearSettingsWidget::onDefineStartByDateClicked);
  m_startDateEdit = new QDateEdit(QDate(2009, 4, 1));
  m_startDateEdit->setCalendarPopup(true);
  dstGridLayout->addWidget(m_startDateEdit, 2, 1);

  // Ends

  auto* endsLabel = new QLabel(tr("Ends"));
  endsLabel->setObjectName("H2");
  dstGridLayout->addWidget(endsLabel, 3, 0);

  auto* dstEndButtonGroup = new QButtonGroup(this);

  m_dayOfWeekAndMonthEndButton = new QRadioButton(tr("Define by Day of The Week And Month"), this);
  dstEndButtonGroup->addButton(m_dayOfWeekAndMonthEndButton);
  dstGridLayout->addWidget(m_dayOfWeekAndMonthEndButton, 4, 0);
  connect(m_dayOfWeekAndMonthEndButton, &QRadioButton::clicked, this, &YearSettingsWidget::onDefineEndByDayWeekMonthClicked);
  m_endWeekBox = new OSComboBox2();
  dstGridLayout->addWidget(m_endWeekBox, 4, 1);

  m_endDayBox = new OSComboBox2();
  dstGridLayout->addWidget(m_endDayBox, 4, 2);

  m_endMonthBox = new OSComboBox2();
  dstGridLayout->addWidget(m_endMonthBox, 4, 3);

  m_dateEndButton = new QRadioButton(tr("Define by Date"), this);
  dstEndButtonGroup->addButton(m_dateEndButton);
  dstGridLayout->addWidget(m_dateEndButton, 5, 0);
  connect(m_dateEndButton, &QRadioButton::clicked, this, &YearSettingsWidget::onDefineEndByDateClicked);
  m_endDateEdit = new QDateEdit(QDate(2009, 10, 1));
  m_endDateEdit->setCalendarPopup(true);
  dstGridLayout->addWidget(m_endDateEdit, 5, 1);

  dstGridLayout->setColumnStretch(4, 10);

  // choices

  m_startWeekBox->addItems(YearSettingsWidget::weeksInMonth());
  m_endWeekBox->addItems(YearSettingsWidget::weeksInMonth());

  m_startDayBox->addItems(YearSettingsWidget::daysOfWeek());
  m_endDayBox->addItems(YearSettingsWidget::daysOfWeek());

  m_startMonthBox->addItems(YearSettingsWidget::months());
  m_endMonthBox->addItems(YearSettingsWidget::months());

  // Stretch

  mainVLayout->addStretch();

  // Default Button State

  m_dateStartButton->setChecked(true);
  m_dateEndButton->setChecked(true);

  // Refresh

  scheduleRefresh();

  // Connect

  m_yearDescription->getImpl<model::detail::YearDescription_Impl>()->onChange.connect<YearSettingsWidget, &YearSettingsWidget::scheduleRefresh>(this);

  connect(OSAppBase::instance(), &OSAppBase::workspaceObjectAddedPtr, this, &YearSettingsWidget::onWorkspaceObjectAdd, Qt::QueuedConnection);

  m_model.getImpl<model::detail::Model_Impl>()->removeWorkspaceObjectPtr.connect<YearSettingsWidget, &YearSettingsWidget::onWorkspaceObjectRemove>(
    this);

  connect(m_startWeekBox, static_cast<void (OSComboBox2::*)(const QString&)>(&OSComboBox2::currentTextChanged), this,
          &YearSettingsWidget::onDstStartDayWeekMonthChanged);
  connect(m_startDayBox, static_cast<void (OSComboBox2::*)(const QString&)>(&OSComboBox2::currentTextChanged), this,
          &YearSettingsWidget::onDstStartDayWeekMonthChanged);
  connect(m_startMonthBox, static_cast<void (OSComboBox2::*)(const QString&)>(&OSComboBox2::currentTextChanged), this,
          &YearSettingsWidget::onDstStartDayWeekMonthChanged);
  connect(m_startDateEdit, &QDateEdit::dateChanged, this, &YearSettingsWidget::dstStartDateChanged);

  connect(m_endWeekBox, static_cast<void (OSComboBox2::*)(const QString&)>(&OSComboBox2::currentTextChanged), this,
          &YearSettingsWidget::onDstEndDayWeekMonthChanged);
  connect(m_endDayBox, static_cast<void (OSComboBox2::*)(const QString&)>(&OSComboBox2::currentTextChanged), this,
          &YearSettingsWidget::onDstEndDayWeekMonthChanged);
  connect(m_endMonthBox, static_cast<void (OSComboBox2::*)(const QString&)>(&OSComboBox2::currentTextChanged), this,
          &YearSettingsWidget::onDstEndDayWeekMonthChanged);
  connect(m_endDateEdit, &QDateEdit::dateChanged, this, &YearSettingsWidget::dstEndDateChanged);
}

bool YearSettingsWidget::calendarYearChecked() {
  if (m_calendarYearButton) {
    return m_calendarYearButton->isChecked();
  } else {
    return false;
  }
}

void YearSettingsWidget::onWorkspaceObjectAdd(std::shared_ptr<openstudio::detail::WorkspaceObject_Impl> wo, const openstudio::IddObjectType& type,
                                              const openstudio::UUID& uuid) {
  if (wo->iddObject().type() == IddObjectType::OS_RunPeriodControl_DaylightSavingTime) {
    wo->onChange.connect<YearSettingsWidget, &YearSettingsWidget::scheduleRefresh>(this);

    scheduleRefresh();
  }
}

void YearSettingsWidget::onWorkspaceObjectRemove(std::shared_ptr<openstudio::detail::WorkspaceObject_Impl> wo, const openstudio::IddObjectType& type,
                                                 const openstudio::UUID& uuid) {
  if (wo->iddObject().type() == IddObjectType::OS_RunPeriodControl_DaylightSavingTime) {
    scheduleRefresh();
  }
}

void YearSettingsWidget::scheduleRefresh() {
  m_dirty = true;

  QTimer::singleShot(0, this, &YearSettingsWidget::refresh);
}

void YearSettingsWidget::refresh() {
  if (m_dirty) {
    m_startWeekBox->blockSignals(true);
    m_startDayBox->blockSignals(true);
    m_startMonthBox->blockSignals(true);
    m_startDateEdit->blockSignals(true);
    m_endWeekBox->blockSignals(true);
    m_endDayBox->blockSignals(true);
    m_endMonthBox->blockSignals(true);
    m_endDateEdit->blockSignals(true);

    // Refresh Select By:

    if (m_yearDescription->calendarYear()) {
      m_calendarYearButton->setChecked(true);

      int index = m_calendarYearEdit->findText(QString::number(m_yearDescription->calendarYear().get()));
      if (index != -1) {
        m_calendarYearEdit->blockSignals(true);
        m_calendarYearEdit->setCurrentIndex(index);
        m_calendarYearEdit->blockSignals(false);
      }

      m_calendarYearEdit->setEnabled(true);

      m_firstDayOfYearEdit->setEnabled(false);
    } else  // First Day of Year
    {
      m_firstDayOfYearButton->setChecked(true);

      m_calendarYearEdit->setEnabled(false);

      unsigned dayOfWeekIndex = 0;

      std::string dayOfWeekString = m_yearDescription->dayofWeekforStartDay();

      std::vector<std::string> dayOfWeekValues = model::YearDescription::validDayofWeekforStartDayValues();

      for (dayOfWeekIndex = 0; dayOfWeekIndex < dayOfWeekValues.size(); dayOfWeekIndex++) {
        if (istringEqual(dayOfWeekValues[dayOfWeekIndex], dayOfWeekString)) {
          break;
        }
      }

      m_firstDayOfYearEdit->blockSignals(true);
      m_firstDayOfYearEdit->setCurrentIndex(dayOfWeekIndex);
      m_firstDayOfYearEdit->blockSignals(false);

      m_firstDayOfYearEdit->setEnabled(true);
    }

    // check if we have a weather file
    // DLM: this is not the right place for this code, however this is an example of how to get the start day
    // of week from the epw file.  The right approach would be to put all this in the model, however the model
    // would have to know the resource path.
    //boost::optional<EpwFile> epwFile;
    //boost::optional<model::WeatherFile> weatherFile = m_model.getOptionalUniqueModelObject<model::WeatherFile>();
    //if (weatherFile) {
    //std::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();
    //openstudio::path resourcesPath = openstudio::toPath(doc->modelTempDir()) / openstudio::toPath("resources");
    //epwFile = weatherFile->file(resourcesPath);
    //}

    // Refresh Daylight Savings Time

    boost::optional<model::RunPeriodControlDaylightSavingTime> dst =
      m_model.getOptionalUniqueModelObject<model::RunPeriodControlDaylightSavingTime>();

    if (dst) {
      m_dstOnOffButton->setChecked(true);
      m_dayOfWeekAndMonthStartButton->setEnabled(true);
      m_dateStartButton->setEnabled(true);
      m_dayOfWeekAndMonthEndButton->setEnabled(true);
      m_dateEndButton->setEnabled(true);

      // DST Start

      Date startDate = dst->startDate();

      QDate qstartDate(startDate.year(), startDate.monthOfYear().value(), startDate.dayOfMonth());

      boost::optional<openstudio::NthDayOfWeekInMonth> startNthDayOfWeekInMonth = dst->startNthDayOfWeekInMonth();

      if (startNthDayOfWeekInMonth) {
        m_dayOfWeekAndMonthStartButton->setChecked(true);

        m_startWeekBox->setEnabled(true);
        m_startWeekBox->setCurrentIndex(startNthDayOfWeekInMonth->value() - 1);

        m_startDayBox->setEnabled(true);
        m_startDayBox->setCurrentIndex(startDate.dayOfWeek().value());

        m_startMonthBox->setEnabled(true);
        m_startMonthBox->setCurrentIndex(startDate.monthOfYear().value() - 1);

        m_startDateEdit->setEnabled(false);
        //m_startDateEdit->setDate(qstartDate);
      } else {
        m_dateStartButton->setChecked(true);

        m_startWeekBox->setEnabled(false);
        //m_startWeekBox->setCurrentIndex(nthDayOfWeekInMonth(startDate).value() - 1);

        m_startDayBox->setEnabled(false);
        //m_startDayBox->setCurrentIndex(startDate.dayOfWeek().value());

        m_startMonthBox->setEnabled(false);
        //m_startMonthBox->setCurrentIndex(startDate.monthOfYear().value() - 1);

        m_startDateEdit->setEnabled(true);
        m_startDateEdit->setDate(qstartDate);
      }

      // DST End

      Date endDate = dst->endDate();

      QDate qendDate(endDate.year(), endDate.monthOfYear().value(), endDate.dayOfMonth());

      boost::optional<openstudio::NthDayOfWeekInMonth> endNthDayOfWeekInMonth = dst->endNthDayOfWeekInMonth();

      if (endNthDayOfWeekInMonth) {
        m_dayOfWeekAndMonthEndButton->setChecked(true);

        m_endWeekBox->setEnabled(true);
        // For some <insert reason>, MonthOfYear::getValues NthDayOfWeekInMonth and starts a 1, while DayOfWeek starts at 0...
        m_endWeekBox->setCurrentIndex(endNthDayOfWeekInMonth->value() - 1);

        m_endDayBox->setEnabled(true);
        m_endDayBox->setCurrentIndex(endDate.dayOfWeek().value());

        m_endMonthBox->setEnabled(true);
        m_endMonthBox->setCurrentIndex(endDate.monthOfYear().value() - 1);

        m_endDateEdit->setEnabled(false);
        //m_endDateEdit->setDate(qendDate);
      } else {
        m_dateEndButton->setChecked(true);

        m_endWeekBox->setEnabled(false);
        //m_endWeekBox->setCurrentIndex(nthDayOfWeekInMonth(endDate).value() - 1);

        m_endDayBox->setEnabled(false);
        //m_endDayBox->setCurrentIndex(endDate.dayOfWeek().value());

        m_endMonthBox->setEnabled(false);
        //m_endMonthBox->setCurrentIndex(endDate.monthOfYear().value() - 1);

        m_endDateEdit->setEnabled(true);
        m_endDateEdit->setDate(qendDate);
      }
    } else {

      m_dstOnOffButton->setChecked(false);
      m_dayOfWeekAndMonthStartButton->setEnabled(false);
      m_dateStartButton->setEnabled(false);
      m_dayOfWeekAndMonthEndButton->setEnabled(false);
      m_dateEndButton->setEnabled(false);

      m_startWeekBox->setEnabled(false);
      m_startDayBox->setEnabled(false);
      m_startMonthBox->setEnabled(false);
      m_endWeekBox->setEnabled(false);
      m_endDayBox->setEnabled(false);
      m_endMonthBox->setEnabled(false);
      m_startDateEdit->setEnabled(false);
      m_endDateEdit->setEnabled(false);
    }

    m_startWeekBox->blockSignals(false);
    m_startDayBox->blockSignals(false);
    m_startMonthBox->blockSignals(false);
    m_startDateEdit->blockSignals(false);
    m_endWeekBox->blockSignals(false);
    m_endDayBox->blockSignals(false);
    m_endMonthBox->blockSignals(false);
    m_endDateEdit->blockSignals(false);

    m_dirty = false;
  }
}

void YearSettingsWidget::onCalendarYearChanged(int index) {
  emit calendarYearSelected(index + FIRSTYEAR);
}

void YearSettingsWidget::onCalendarYearButtonClicked() {
  int index = m_calendarYearEdit->currentIndex();

  emit calendarYearSelected(index + FIRSTYEAR);
}

void YearSettingsWidget::onFirstDayofYearClicked() {
  emit firstDayofYearSelected(m_firstDayOfYearEdit->currentText());
}

void YearSettingsWidget::onDstStartDayWeekMonthChanged() {

  NthDayOfWeekInMonth _nth(m_startWeekBox->currentIndex() + 1);  // NthDayOfWeekInMonth starts at 1...

  DayOfWeek _dayOfWeek(m_startDayBox->currentIndex());
  MonthOfYear _monthOfYear(m_startMonthBox->currentIndex() + 1);  // MonthOfYear starts at 1...

  emit dstStartDayOfWeekAndMonthChanged(_nth.value(), _dayOfWeek.value(), _monthOfYear.value());
}

void YearSettingsWidget::onDstEndDayWeekMonthChanged() {

  NthDayOfWeekInMonth _nth(m_endWeekBox->currentIndex() + 1);  // NthDayOfWeekInMonth starts at 1...

  DayOfWeek _dayOfWeek(m_endDayBox->currentIndex());
  MonthOfYear _monthOfYear(m_endMonthBox->currentIndex() + 1);  // MonthOfYear starts at 1...

  emit dstEndDayOfWeekAndMonthChanged(_nth.value(), _dayOfWeek.value(), _monthOfYear.value());
}

QStringList YearSettingsWidget::weeksInMonth() {
  QStringList result = {tr("First"), tr("Second"), tr("Third"), tr("Fourth"), tr("Last")};
  return result;
}

QStringList YearSettingsWidget::validDayofWeekforStartDay() {

  // Similar to model::YearDescription::validDayofWeekforStartDayValues(), but we need to be able to translate them
  QStringList result = {tr("Sunday"),   tr("Monday"), tr("Tuesday"),  tr("Wednesday"),
                        tr("Thursday"), tr("Friday"), tr("Saturday"), tr("UseWeatherFile")};

  return result;
}
QStringList YearSettingsWidget::daysOfWeek() {
  QStringList result = {tr("Monday"), tr("Tuesday"), tr("Wednesday"), tr("Thursday"), tr("Friday"), tr("Saturday"), tr("Sunday")};

  return result;
}

QStringList YearSettingsWidget::months() {
  QStringList result = {tr("January"), tr("February"), tr("March"),     tr("April"),   tr("May"),      tr("June"),
                        tr("July"),    tr("August"),   tr("September"), tr("October"), tr("November"), tr("December")};

  return result;
}

void YearSettingsWidget::onDefineStartByDayWeekMonthClicked() {
  m_startWeekBox->setEnabled(true);

  m_startDayBox->setEnabled(true);

  m_startMonthBox->setEnabled(true);

  m_startDateEdit->setEnabled(false);

  onDstStartDayWeekMonthChanged();
}

void YearSettingsWidget::onDefineEndByDayWeekMonthClicked() {
  m_endWeekBox->setEnabled(true);

  m_endDayBox->setEnabled(true);

  m_endMonthBox->setEnabled(true);

  m_endDateEdit->setEnabled(false);

  onDstEndDayWeekMonthChanged();
}

void YearSettingsWidget::onDefineStartByDateClicked() {
  m_startWeekBox->setEnabled(false);

  m_startDayBox->setEnabled(false);

  m_startMonthBox->setEnabled(false);

  m_startDateEdit->setEnabled(true);

  emit dstStartDateChanged(m_startDateEdit->date());
}

void YearSettingsWidget::onDefineEndByDateClicked() {
  m_endWeekBox->setEnabled(false);

  m_endDayBox->setEnabled(false);

  m_endMonthBox->setEnabled(false);

  m_endDateEdit->setEnabled(true);

  emit dstEndDateChanged(m_endDateEdit->date());
}

}  // namespace openstudio
