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

#ifndef OPENSTUDIO_LIBRARYTABWIDGET_HPP
#define OPENSTUDIO_LIBRARYTABWIDGET_HPP

#include <QWidget>
#include <vector>

class QStackedWidget;

class QPushButton;

namespace openstudio {

class LibraryTabWidget : public QWidget
{
  Q_OBJECT

 public:
  explicit LibraryTabWidget(QWidget* parent = nullptr);

  virtual ~LibraryTabWidget() {}

  void showRemoveButton();

  void hideRemoveButton();

  void addTab(QWidget* widget, const QString& selectedImagePath, const QString& unSelectedImagePath);

  /* This method, given a tab widget, will change the currentIndex to be the following if there is one, or zero otherwise
   * It will hide the tab by hide the corresponding button.
   * Useful to hide the LoopChooserView for components that don't have a water coil for eg. */
  void hideTab(QWidget* widget, bool hide = true);

 signals:

  void removeButtonClicked(bool);

 public slots:

  void setCurrentIndex(int index);

  void setCurrentWidget(QWidget* widget);

 private slots:

  void select();

 private:
  QStackedWidget* m_pageStack;

  QWidget* m_tabBar;

  std::vector<QString> m_selectedPixmaps;

  std::vector<QString> m_neighborSelectedPixmaps;

  std::vector<QString> m_unSelectedPixmaps;

  std::vector<QPushButton*> m_tabButtons;

  QPushButton* m_removeButton = nullptr;
};

}  // namespace openstudio

#endif  // OPENSTUDIO_LIBRARYTABWIDGET_HPP
