#include <CQColorsEditList.h>
#include <CQColors.h>
#include <CQColorsTheme.h>
#include <CQColorsPalette.h>

#include <CQPixmapCache.h>
#include <CQGroupBox.h>
#include <CQColorEdit.h>
#include <CQUtil.h>

#include <svg/up_top_svg.h>
#include <svg/up_svg.h>
#include <svg/down_svg.h>
#include <svg/left_svg.h>
#include <svg/right_svg.h>

#include <QComboBox>
#include <QListWidget>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <set>

#if 0
namespace {

void
addGridLabelWidget(QGridLayout *playout, const QString &label, QWidget *widget, int &row)
{
  QLabel *qlabel = CQUtil::makeLabelWidget<QLabel>(label, "label" + label);

  playout->addWidget(qlabel, row, 0);
  playout->addWidget(widget, row, 1);

  ++row;
}

}
#endif

//---

CQColorsEditList::
CQColorsEditList(QWidget *parent) :
 QFrame(parent)
{
  QVBoxLayout *layout = CQUtil::makeLayout<QVBoxLayout>(this, 2, 2);

  //----

  // themes combo frame
  QFrame *themeFrame = CQUtil::makeWidget<QFrame>("themeFrame");

  layout->addWidget(themeFrame);

  QHBoxLayout *themeLayout = CQUtil::makeLayout<QHBoxLayout>(themeFrame, 2, 2);

  QLabel *themesLabel = CQUtil::makeLabelWidget<QLabel>("Theme", "themesLabel");

  themeLayout->addWidget(themesLabel);

  themesCombo_ = CQUtil::makeWidget<QComboBox>("themesCombo");

  themesCombo_->setToolTip("Select Theme to Edit");

  connect(themesCombo_, SIGNAL(currentIndexChanged(int)), this, SLOT(themesComboSlot(int)));

  themeLayout->addWidget(themesCombo_);

  themeLayout->addStretch(1);

  //----

  QFrame *listFrame = CQUtil::makeWidget<QFrame>("listFrame");

  QHBoxLayout *listLayout = CQUtil::makeLayout<QHBoxLayout>(listFrame, 2, 2);

  layout->addWidget(listFrame);

  //----

  // list of palettes for current theme
  CQGroupBox *currentGroup =
    CQUtil::makeLabelWidget<CQGroupBox>("Palettes", "currentGroup");

  QVBoxLayout *currentGroupLayout = CQUtil::makeLayout<QVBoxLayout>(currentGroup, 2, 2);

  listLayout->addWidget(currentGroup);

  //--

  currentList_ = CQUtil::makeWidget<QListWidget>("currentList");

  currentList_->setToolTip("List of Theme Palettes");

  currentGroupLayout->addWidget(currentList_);

  //----

  // control frame (move up/down, transfer left/right)
  QFrame *buttonsFrame = CQUtil::makeWidget<QFrame>("buttonsFrame");

  listLayout->addWidget(buttonsFrame);

  QVBoxLayout *controlLayout = CQUtil::makeLayout<QVBoxLayout>(buttonsFrame, 2, 2);

  auto addToolButton = [&](const QString &name, const QString &iconName, const char *slotName) {
    QToolButton *button = CQUtil::makeWidget<QToolButton>(name);

    button->setIcon(CQPixmapCacheInst->getIcon(iconName));

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  QToolButton *upTopButton = addToolButton("upTip", "UP_TOP", SLOT(upTopSlot()));
  QToolButton *upButton    = addToolButton("up"   , "UP"    , SLOT(upSlot   ()));
  QToolButton *downButton  = addToolButton("down" , "DOWN"  , SLOT(downSlot ()));
  QToolButton *leftButton  = addToolButton("left" , "LEFT"  , SLOT(leftSlot ()));
  QToolButton *rightButton = addToolButton("right", "RIGHT" , SLOT(rightSlot()));

  upTopButton->setToolTip("Move palette to top of theme list");
  upButton   ->setToolTip("Move palette up in theme list");
  downButton ->setToolTip("Move palette down in theme list");
  leftButton ->setToolTip("Add palette to theme list (from unused)");
  rightButton->setToolTip("Remove palette from theme list");

  controlLayout->addStretch(1);
  controlLayout->addWidget(upTopButton);
  controlLayout->addWidget(upButton);
  controlLayout->addWidget(downButton);
  controlLayout->addStretch(1);
  controlLayout->addWidget(leftButton);
  controlLayout->addWidget(rightButton);
  controlLayout->addStretch(1);

  //----

  // list of unused palettes
  CQGroupBox *allGroup =
    CQUtil::makeLabelWidget<CQGroupBox>("Unused Palettes", "allGroup");

  QVBoxLayout *allGroupLayout = CQUtil::makeLayout<QVBoxLayout>(allGroup, 2, 2);

  listLayout->addWidget(allGroup);

  //--

  allList_ = CQUtil::makeWidget<QListWidget>("allList");

  allList_->setToolTip("List of Unused Palettes");

  allGroupLayout->addWidget(allList_);

  //---

  QFrame *dataFrame = CQUtil::makeWidget<QFrame>("dataFrame");

  QHBoxLayout *dataLayout = CQUtil::makeLayout<QHBoxLayout>(dataFrame);

  layout->addWidget(dataFrame);

  QFrame *colorsFrame = CQUtil::makeWidget<QFrame>("colorsFrame");

  dataLayout->addWidget(colorsFrame);

  //---

#if 0
  QGridLayout *colorsLayout = CQUtil::makeLayout<QGridLayout>(colorsFrame, 2, 2);

  int row = 0;

  selectColorEdit_ = CQUtil::makeWidget<CQColorEdit>("selectColorEdit");
  insideColorEdit_ = CQUtil::makeWidget<CQColorEdit>("insideColorEdit");

  selectColorEdit_->setToolTip("Set Selected Color");
  insideColorEdit_->setToolTip("Set Inside Color");

  addGridLabelWidget(colorsLayout, "Select Color", selectColorEdit_, row);
  addGridLabelWidget(colorsLayout, "Inside Color", insideColorEdit_, row);

  connect(selectColorEdit_, SIGNAL(colorChanged(const QColor &)),
          this, SLOT(selectColorSlot(const QColor &)));
  connect(insideColorEdit_, SIGNAL(colorChanged(const QColor &)),
          this, SLOT(insideColorSlot(const QColor &)));
#endif

  //---

  connect(CQColorsMgrInst, SIGNAL(themesChanged()), this, SLOT(updateThemes()));
  connect(CQColorsMgrInst, SIGNAL(palettesChanged()), this, SLOT(updateLists()));

  updateThemes();
  updateLists();
  updateData();
}

void
CQColorsEditList::
connectSlots(bool connect)
{
  if (connect) {
    this->connect(CQColorsMgrInst, SIGNAL(themesChanged()), this, SLOT(updateThemes()));
    this->connect(CQColorsMgrInst, SIGNAL(palettesChanged()), this, SLOT(updateLists()));
  }
  else {
    this->disconnect(CQColorsMgrInst, SIGNAL(themesChanged()), this, SLOT(updateThemes()));
    this->disconnect(CQColorsMgrInst, SIGNAL(palettesChanged()), this, SLOT(updateLists()));
  }
}

void
CQColorsEditList::
themesComboSlot(int)
{
  updateLists();
  updateData();
}

void
CQColorsEditList::
updateThemes()
{
  QStringList names;

  CQColorsMgrInst->getThemeNames(names);

  themesCombo_->clear();

  themesCombo_->addItems(names);
}

void
CQColorsEditList::
updateLists()
{
  currentList_->clear();
  allList_    ->clear();

  //---

  CQColorsTheme *theme = currentTheme();
  if (! theme) return;

  //---

  using StringSet = std::set<QString>;

  StringSet stringSet;

  int n = theme->numPalettes();

  QStringList names;

  for (int i = 0; i < n; ++i) {
    const QString &name = theme->palette(i)->name();

    names << name;

    stringSet.insert(name);
  }

  currentList_->addItems(names);

  if (currentList_->count())
    currentList_->setCurrentItem(currentList_->item(0), QItemSelectionModel::Select);

  //---

  QStringList allNames;

  CQColorsMgrInst->getPaletteNames(allNames);

  QStringList otherNames;

  for (int i = 0; i < allNames.length(); ++i) {
    const QString &name = allNames[i];

    if (stringSet.find(name) == stringSet.end())
      otherNames << name;
  }

  allList_->addItems(otherNames);

  if (allList_->count())
    allList_->setCurrentItem(allList_->item(0), QItemSelectionModel::Select);
}

void
CQColorsEditList::
updateData()
{
  CQColorsTheme *theme = currentTheme();
  if (! theme) return;

#if 0
  selectColorEdit_->setColor(theme->selectColor());
  insideColorEdit_->setColor(theme->insideColor());
#endif
}

// move current item up
void
CQColorsEditList::
upTopSlot()
{
  upSlot(/*top*/true);
}

// move current item up
void
CQColorsEditList::
upSlot(bool top)
{
  CQColorsTheme *theme = currentTheme();
  if (! theme) return;

  //---

  QListWidgetItem *item1;
  int              row1;

  if (! getCurrentItem(item1, row1))
    return;

  if (row1 <= 0)
    return;

  //---

  connectSlots(false);

  int row0 = (! top ? row1 - 1 : 0);

  QListWidgetItem *item0 = currentList_->item(row0);

  QString name0 = item0->text();
  QString name1 = item1->text();

  item0->setText(name1);
  item1->setText(name0);

  theme->movePalette(name0, row1);
  theme->movePalette(name1, row0);

  item0 = currentList_->item(row0);

  currentList_->setCurrentItem(item0, QItemSelectionModel::ClearAndSelect);

  connectSlots(true);

  emit palettesChanged();
}

// move current item down
void
CQColorsEditList::
downSlot()
{
  CQColorsTheme *theme = currentTheme();
  if (! theme) return;

  //---

  QListWidgetItem *item1;
  int              row1;

  if (! getCurrentItem(item1, row1))
    return;

  if (row1 >= currentList_->count() - 1)
    return;

  //---

  connectSlots(false);

  int row2 = row1 + 1;

  QListWidgetItem *item2 = currentList_->item(row2);

  QString name1 = item1->text();
  QString name2 = item2->text();

  item1->setText(name2);
  item2->setText(name1);

  theme->movePalette(name1, row2);
  theme->movePalette(name2, row1);

  item2 = currentList_->item(row2);

  currentList_->setCurrentItem(item2, QItemSelectionModel::ClearAndSelect);

  connectSlots(true);

  emit palettesChanged();
}

// move all item to current
void
CQColorsEditList::
leftSlot()
{
  CQColorsTheme *theme = currentTheme();
  if (! theme) return;

  //---

  QListWidgetItem *item;
  int              row;

  if (! getAllItem(item, row))
    return;

  //---

  connectSlots(false);

  QString name = item->text();

  item = allList_->takeItem(row);

  delete item;

  theme->addNamedPalette(name);

  currentList_->addItem(name);

  item = currentList_->item(currentList_->count() - 1);

  currentList_->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);

  if (row > 0)
    item = allList_->item(row - 1);
  else
    item = allList_->item(row);

  allList_->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);

  connectSlots(true);

  emit palettesChanged();
}

// move current item to all
void
CQColorsEditList::
rightSlot()
{
  CQColorsTheme *theme = currentTheme();
  if (! theme) return;

  //---

  // can't empty list !
  if (currentList_->count() <= 1)
    return;

  //---

  QListWidgetItem *item;
  int              row;

  if (! getCurrentItem(item, row))
    return;

  //---

  connectSlots(false);

  QString name = item->text();

  item = currentList_->takeItem(row);

  delete item;

  theme->removeNamedPalette(name);

  allList_->addItem(name);

  item = allList_->item(allList_->count() - 1);

  allList_->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);

  if (row > 0)
    item = currentList_->item(row - 1);
  else
    item = currentList_->item(row);

  currentList_->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);

  connectSlots(true);

  emit palettesChanged();
}

void
CQColorsEditList::
selectColorSlot(const QColor &c)
{
#if 0
  CQColorsTheme *theme = currentTheme();
  if (! theme) return;

  theme->setSelectColor(c);
#else
  Q_UNUSED(c)
#endif
}

void
CQColorsEditList::
insideColorSlot(const QColor &c)
{
#if 0
  CQColorsTheme *theme = currentTheme();
  if (! theme) return;

  theme->setInsideColor(c);
#else
  Q_UNUSED(c)
#endif
}

bool
CQColorsEditList::
getCurrentItem(QListWidgetItem* &item, int &row) const
{
  item = nullptr;
  row  = -1;

  QList<QListWidgetItem *> selected = currentList_->selectedItems();
  if (! selected.length()) return false;

  item = selected[0];

  row = currentList_->row(item);

  return true;
}

bool
CQColorsEditList::
getAllItem(QListWidgetItem* &item, int &row) const
{
  item = nullptr;
  row  = -1;

  QList<QListWidgetItem *> selected = allList_->selectedItems();
  if (! selected.length()) return false;

  item = selected[0];

  row = allList_->row(item);

  return true;
}

CQColorsTheme *
CQColorsEditList::
currentTheme() const
{
  QString name = themesCombo_->currentText();

  CQColorsTheme *theme = CQColorsMgrInst->getNamedTheme(name);

  return theme;
}
