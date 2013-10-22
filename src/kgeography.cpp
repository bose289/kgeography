/***************************************************************************
 *   Copyright (C) 2004-2007 by Albert Astals Cid                          *
 *   aacid@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "kgeography.h"

#include <kaction.h>
#include <kdialog.h>
#include <klocale.h>
#include <kconfigdialog.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kmenubar.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qsize.h>
#include <qtimer.h>

#include "answersdialog.h"
#include "capitaldivisionasker.h"
#include "divisioncapitalasker.h"
#include "divisionflagasker.h"
#include "flagdivisionasker.h"
#include "settings.h"
#include "mapasker.h"
#include "mapchooser.h"
#include "mapparser.h"
#include "map.h"
#include "placeasker.h"
#include "integerinputdialog.h"
#include "ui_kgeographyoptions.h"

kgeography::kgeography() : KXmlGuiWindow(), p_firstShow(true), p_mustShowResultsDialog(false)
{
	p_map = 0;
	p_askWidget = 0;

	p_bigWidget = new QWidget(this);
	QHBoxLayout *bigWidgetLayout = new QHBoxLayout(p_bigWidget);
	bigWidgetLayout -> setSpacing(0);
	bigWidgetLayout -> setMargin(0);

	QWidget *leftWidget = new QWidget(p_bigWidget);
	QVBoxLayout *leftWidgetLayout = new QVBoxLayout(leftWidget);
// 	leftWidgetLayout -> setSpacing(0);
// 	leftWidgetLayout -> setMargin(0);

	p_currentMap = new QLabel(leftWidget);
	p_currentMap -> setAlignment(Qt::AlignCenter);
	p_consult = new KPushButton(i18n("E&xplore Map"), leftWidget);
	p_consult->setWhatsThis(i18n("In this section left click on any part of the map to learn about the divisions" ));
	QLabel * testYourselfLabel = new QLabel( i18n("Test Yourself:"), this );
	testYourselfLabel->setAlignment( Qt::AlignHCenter );
	p_askMap = new KPushButton(i18n("&Location of Regions"), leftWidget);
	p_askMap->setWhatsThis(i18n("In this challenge you are given a division name on the left under the menu and you must find it on the map and click on it"));
	p_askPlaceMap = new KPushButton(i18n("&Place Regions in the Map"), leftWidget);
	p_askPlaceMap->setWhatsThis(i18n("In this challenge you are given the shape of a region and you must place it on the map"));
	p_askCapitalDivisions = new KPushButton(i18n("&Regions by Capital"), leftWidget);
	p_askCapitalDivisions->setWhatsThis(i18n("In this quiz you have to guess the division name given its capital"));
	p_askDivisionCapitals = new KPushButton(i18n("&Capitals of Regions"), leftWidget);
	p_askDivisionCapitals->setWhatsThis(i18n("In this quiz you have to guess the capital of a given division name"));
	p_askFlagDivisions = new KPushButton(i18n("Re&gions by Flag"), leftWidget);
	p_askFlagDivisions->setWhatsThis(i18n("In this quiz you have to guess the division name given its flag"));
	p_askDivisionFlags = new KPushButton(i18n("&Flags of Regions"), leftWidget);
	p_askDivisionFlags->setWhatsThis(i18n("In this quiz you have to guess the flag of a division given its name"));
	p_underLeftWidget = new QWidget(leftWidget);
	// where the number of answers will be shown
	QVBoxLayout *underLeftWidgetLayout = new QVBoxLayout(p_underLeftWidget);
	underLeftWidgetLayout -> setSpacing(KDialog::spacingHint());
	underLeftWidgetLayout -> setMargin(KDialog::marginHint());

	leftWidgetLayout -> addWidget(p_currentMap);
	leftWidgetLayout -> addWidget(p_consult);
	leftWidgetLayout -> addSpacing(10);
	leftWidgetLayout -> addWidget(testYourselfLabel);
	leftWidgetLayout -> addWidget(p_askMap);
	leftWidgetLayout -> addWidget(p_askPlaceMap);
	leftWidgetLayout -> addWidget(p_askCapitalDivisions);
	leftWidgetLayout -> addWidget(p_askDivisionCapitals);
	leftWidgetLayout -> addWidget(p_askFlagDivisions);
	leftWidgetLayout -> addWidget(p_askDivisionFlags);
	leftWidgetLayout -> addWidget(p_underLeftWidget);

	// Make p_underLeftWidget as thin as possible, and always be shown in the same place
	QWidget* p_underUnderLeftWidget = new QWidget(leftWidget);
	QVBoxLayout *underUnderLeftWidgetLayout = new QVBoxLayout(p_underUnderLeftWidget);
	underUnderLeftWidgetLayout -> addStretch(1);
	leftWidgetLayout -> addWidget(p_underUnderLeftWidget);

	bigWidgetLayout -> addWidget(leftWidget);
	setCentralWidget(p_bigWidget);

	connect(p_consult, SIGNAL(clicked()), this, SLOT(consult()));
	connect(p_askMap, SIGNAL(clicked()), this, SLOT(askMap()));
	connect(p_askPlaceMap, SIGNAL(clicked()), this, SLOT(askPlaceMap()));
	connect(p_askCapitalDivisions, SIGNAL(clicked()), this, SLOT(askCapitalDivisions()));
	connect(p_askDivisionCapitals, SIGNAL(clicked()), this, SLOT(askDivisionCapitals()));
	connect(p_askFlagDivisions, SIGNAL(clicked()), this, SLOT(askFlagDivisions()));
	connect(p_askDivisionFlags, SIGNAL(clicked()), this, SLOT(askDivisionFlags()));

	QAction *a = KStandardAction::open(this, SLOT(openMap()), actionCollection());
	a -> setText(i18n("&Open Map..."));

	a = KStandardAction::quit(this, SLOT(close()), actionCollection());
	
	p_zoom =  actionCollection()->add<KToggleAction>( "zoom_select" );
	p_zoom -> setText(i18n("&Zoom") );
	p_zoom -> setIcon( KIcon("zoom-in") );
	p_zoom -> setEnabled(false);

	p_zoomOriginal = actionCollection()->addAction( "zoom_original" );
	p_zoomOriginal -> setText( i18n("&Original Size") );
	p_zoomOriginal -> setIcon( KIcon("zoom-original") );
	p_zoomOriginal -> setEnabled(false);

	p_zoomAutomatic = actionCollection()->add<KToggleAction>( "zoom_automatic" );
	p_zoomAutomatic -> setText(  i18n("&Automatic Zoom") );
	p_zoomAutomatic -> setIcon( KIcon("zoom-fit-best") );
	p_zoomAutomatic -> setEnabled(false);

	p_move = actionCollection()->add<KToggleAction>( "move" );
	p_move->setText( i18n("&Move") );
	p_move->setIcon( KIcon("transform-move") );
	p_move -> setEnabled(false);

	a = actionCollection()->addAction( "disclaimer" );
	a->setText( i18n("Disclaimer") );
	connect(a, SIGNAL(triggered()), this, SLOT(disclaimer()));
	
	p_showAuthor = actionCollection()->addAction( "author" );
	p_showAuthor->setText( i18n("Map author") );
	p_showAuthor->setEnabled(false);
	connect(p_showAuthor, SIGNAL(triggered()), this, SLOT(showMapAuthor()));

	KStandardAction::preferences(this, SLOT(showPreferencesDialog()), actionCollection());

	setupGUI(Keys | ToolBar | Save | Create);

	show();
}

kgeography::~kgeography()
{
	delete p_askWidget;
	delete p_map;
}

void kgeography::showPreferencesDialog()
{
	if ( KConfigDialog::showDialog( "settings" ) )
		return; 

	// KConfigDialog didn't find an instance of this dialog, so lets
	// create it : 
	KConfigDialog* dialog = new KConfigDialog(this, "settings",
											  kgeographySettings::self()); 
        dialog->setHelp("configuration", "kgeography");
	QWidget *w = new QWidget();
	Ui::Options confWdg;
	confWdg.setupUi(w);

	dialog->addPage(w, i18n("General"), "kgeography" ); 

	// User edited the configuration - update your local copies of the 
	// configuration data 
	connect(dialog, SIGNAL(settingsChanged(const QString&)), 
			this, SLOT(updateConfiguration()) ); 

	dialog->show();
}

void kgeography::updateConfiguration()
{
	boxAsker *aBoxAsker = dynamic_cast<boxAsker*>(p_askWidget);
	if ( aBoxAsker != NULL ) {
		aBoxAsker->updateLayout();
	}
}

void kgeography::showEvent(QShowEvent *)
{
	if (p_firstShow)
	{
		QString file = kgeographySettings::self() -> lastMap();

		if (QFile::exists(file))
		{
			mapReader reader;
			KGmap *kgmap = reader.parseMap(file);
			if (kgmap)
			{
				setMap(kgmap);
			}
			else
			{
				KMessageBox::error(this, i18n("Could not open last used map. Error parsing %1: %2", file, reader.getError()));
				openMap();
			}
		}
		else openMap();

		if (!p_map)
		{
			p_currentMap -> setText(i18n("There is no current map"));
			p_consult -> setEnabled(false);
			p_askMap -> setEnabled(false);
			p_askPlaceMap -> setEnabled(false);
			p_askFlagDivisions -> setEnabled(false);
			p_askDivisionFlags -> setEnabled(false);
			p_askCapitalDivisions -> setEnabled(false);
			p_askDivisionCapitals -> setEnabled(false);
		}
		// if anyone can explain why with the slot works and now without
		// i'll be glad to learn
		QTimer::singleShot(0, this, SLOT(resizeMainWindow()));
// 		resizeMainWindow();

		p_firstShow = false;
	}
}

void kgeography::openMap()
{
	mapChooser mp(this);
	if (mp.exec() == mapChooser::Accepted)
	{
		showResultsDialog();
		setMap(mp.getMap());
		// if anyone can explain why with the slot works and now without
		// i'll be glad to learn
		QTimer::singleShot(0, this, SLOT(resizeMainWindow()));
// 		resizeMainWindow();
	}
}

void kgeography::consult()
{
	// No need to create another map viewer if we are already in it
	mapAsker *ma = dynamic_cast<mapAsker*>(p_askWidget);
	if (ma && !ma -> isAsker()) return;

	showResultsDialog();
	removeOldAskWidget();
	p_askWidget = new mapAsker(p_bigWidget, p_map, p_underLeftWidget, false);
	p_zoom -> setEnabled(true);
	p_zoomOriginal -> setEnabled(true);
	p_zoomAutomatic -> setEnabled(true);
	putAskWidget();
}

int kgeography::askQuestionNumber(int upto, bool *rOK)
{
	int i;
	bool ok;
	i = IntegerInputDialog::GetInteger(this, i18n("Number of Questions"),
					   i18n("How many questions do you want? (1 to %1)", upto),
					   1, upto, upto, &ok);
	if ( rOK != NULL ) *rOK = ok;
	return i;
}

void kgeography::askCapitalDivisions()
{
	int i;
	bool ok;
	showResultsDialog();
	i = askQuestionNumber(p_map -> count(division::eCapital), &ok);
	if (ok)
	{
		removeOldAskWidget();
		p_askWidget = new capitalDivisionAsker(p_bigWidget, p_map, p_underLeftWidget, i);
		putAskWidget();
		p_mustShowResultsDialog = true;
	}
	else consult();
}

void kgeography::askDivisionCapitals()
{
	int i;
	bool ok;
	showResultsDialog();
	i = askQuestionNumber(p_map -> count(division::eCapital), &ok);
	if (ok)
	{
		removeOldAskWidget();
		p_askWidget = new divisionCapitalAsker(p_bigWidget, p_map, p_underLeftWidget, i);
		putAskWidget();
		p_mustShowResultsDialog = true;
	}
	else consult();
}

void kgeography::askMap()
{
	int i;
	bool ok;
	showResultsDialog();
	i = askQuestionNumber(p_map -> count(division::eClick), &ok);
	if (ok)
	{
		removeOldAskWidget();
		p_askWidget = new mapAsker(p_bigWidget, p_map, p_underLeftWidget, true, i);
		p_zoom -> setEnabled(true);
		p_zoomOriginal -> setEnabled(true);
		p_zoomAutomatic -> setEnabled(true);
		putAskWidget();
		p_mustShowResultsDialog = true;
	}
	else consult();
}

void kgeography::askPlaceMap()
{
	int i;
	bool ok;
	showResultsDialog();
	i = askQuestionNumber(p_map -> count(division::eClick), &ok);
	if (ok)
	{
		removeOldAskWidget();
		p_askWidget = new placeAsker(p_bigWidget, p_map, p_underLeftWidget, i);
		p_zoom -> setEnabled(true);
		p_zoomOriginal -> setEnabled(true);
		p_zoomAutomatic -> setEnabled(true);
		putAskWidget();
		p_mustShowResultsDialog = true;
	}
	else consult();
}

void kgeography::askFlagDivisions()
{
	int i;
	bool ok;
	showResultsDialog();
	i = askQuestionNumber(p_map -> count(division::eFlag), &ok);
	if (ok)
	{
		removeOldAskWidget();
		p_askWidget = new flagDivisionAsker(p_bigWidget, p_map, p_underLeftWidget, i);
		putAskWidget();
		p_mustShowResultsDialog = true;
	}
	else consult();
}

void kgeography::askDivisionFlags()
{
	int i;
	bool ok;
	showResultsDialog();
	i = askQuestionNumber(p_map -> count(division::eFlag), &ok);
	if (ok)
	{
		removeOldAskWidget();
		p_askWidget = new divisionFlagAsker(p_bigWidget, p_map, p_underLeftWidget, i);
		putAskWidget();
		p_mustShowResultsDialog = true;
	}
	else consult();
}

void kgeography::removeOldAskWidget()
{
	// next lines because setZoom(true) (may have) put cursor at application level
	if ( p_askWidget != NULL )
		p_askWidget->setZoom(false);
	delete p_askWidget;
	p_askWidget = 0;
	p_zoom -> setEnabled(false);
	p_zoomOriginal -> setEnabled(false);
	p_zoomAutomatic -> setEnabled(false);
	p_move -> setEnabled(false);
	p_zoom -> setChecked(false);
	p_move -> setChecked(false);
}

QSize kgeography::getPreferredSize()
{
	int ySize = menuBar() -> size().height() + toolBar() -> size().height() + ((mapAsker*) p_askWidget)->mapSize().height();
	int xSize = p_underLeftWidget -> parentWidget()-> size().width() + ((mapAsker*) p_askWidget)->mapSize().width();
	return QSize(xSize + 10, ySize + 10);
}

void kgeography::putAskWidget()
{
	static_cast<QBoxLayout*>(p_bigWidget -> layout()) -> addWidget(p_askWidget, 1);
	p_askWidget -> setAutomaticZoom(p_zoomAutomatic -> isChecked());
	p_askWidget -> show();
	connect(p_askWidget, SIGNAL(setZoomActionChecked(bool)), p_zoom, SLOT(setChecked(bool)));
	connect(p_zoom, SIGNAL(toggled(bool)), p_askWidget, SLOT(setZoom(bool)));
	connect(p_zoomOriginal, SIGNAL(triggered()), p_askWidget, SLOT(setOriginalZoom()));
	connect(p_zoomAutomatic, SIGNAL(toggled(bool)), this, SLOT(setAutomaticZoom(bool)));
	connect(p_askWidget, SIGNAL(setMoveActionEnabled(bool)), this, SLOT(setMoveActionEnabled(bool)));
	connect(p_askWidget, SIGNAL(setMoveActionChecked(bool)), p_move, SLOT(setChecked(bool)));
	connect(p_move, SIGNAL(toggled(bool)), p_askWidget, SLOT(setMovement(bool)));
	connect(p_askWidget, SIGNAL(questionsEnded()), this, SLOT(questionsEnded()));
}

void kgeography::setMap(KGmap *m)
{
	removeOldAskWidget();
	kgeographySettings *set = kgeographySettings::self();
	set -> setLastMap(m -> getFile());
	set -> writeConfig();
	delete p_map;
	p_map = m;
	p_showAuthor->setEnabled(m);

	QString mapName = i18nc(p_map -> getFileName().toUtf8(), p_map -> getName().toUtf8());
	QString divisionType = i18nc(p_map -> getFileName().toUtf8(), p_map->getDivisionsString().toUtf8());

	p_askMap->setText(i18n("&Location of %1", divisionType));
	p_askPlaceMap->setText(i18n("&Place %1 in the Map", divisionType));
	p_askCapitalDivisions->setText(i18nc("@title", p_map->getCapitalToDivisionTitle().toUtf8()));
	p_askDivisionCapitals->setText(i18nc("@title", p_map->getDivisionToCapitalTitle().toUtf8()));
	p_askFlagDivisions->setText(i18n("%1 by Flag", divisionType));
	p_askDivisionFlags->setText(i18n("&Flags of %1", divisionType));

	p_currentMap -> setText(i18n("<qt>Current map:<br /><b>%1</b></qt>", mapName));
	p_consult -> setEnabled(true);
	p_askMap -> setEnabled(m -> count(division::eClick) > 0);
	p_askPlaceMap -> setEnabled(m -> count(division::eClick) > 0);
	// as we always want to let user choose among 4 choices, verify we have the amunitions
	p_askFlagDivisions -> setEnabled(m -> count(division::eFlag) > 3);
	p_askDivisionFlags -> setEnabled(m -> count(division::eFlag) > 3);
	p_askCapitalDivisions -> setEnabled(m -> count(division::eCapital) > 3);
	p_askDivisionCapitals -> setEnabled(m -> count(division::eCapital) > 3);
	consult();

	// set a maximum width for the under left widget, enabling the labels wordwrap and
	// avoiding the move of the map in case of large division names.
	p_underLeftWidget->setMaximumWidth(static_cast<QWidget*>(p_underLeftWidget->parent())->width()- (KDialog::marginHint()+KDialog::spacingHint())*2 );
}

void kgeography::disclaimer()
{
	KMessageBox::information(this, i18n("Maps, flags, translations, etc. are as accurate as their respective authors could achieve,"
					    " but KGeography should not be taken as an authoritative source."), i18n("Disclaimer"));
}

void kgeography::showMapAuthor()
{
	if (p_map)
	{
		KMessageBox::information(this, i18n("This map has been created by %1.", p_map->getAuthor()), i18n("Map Author"));
	}
}

void kgeography::resizeMainWindow()
{
	if (p_askWidget && width() ) resize(getPreferredSize().expandedTo(size()));
}

void kgeography::questionsEnded()
{
	showResultsDialog();
	consult();
}

void kgeography::setAutomaticZoom(bool b)
{
	p_zoom->setChecked(false);
	p_askWidget->setAutomaticZoom(b);
}

void kgeography::setMoveActionEnabled(bool b)
{
	p_move->setEnabled(b);
	if (b && p_zoomAutomatic->isChecked())
	{
		// we don't want the unchecking to bring us to the original zoom state
		p_zoomAutomatic->blockSignals(true);
		p_zoomAutomatic->setChecked(false);
		p_zoomAutomatic->blockSignals(false);
	}
}

void kgeography::showResultsDialog()
{
	if (p_mustShowResultsDialog)
	{
		p_mustShowResultsDialog = false;
		int ca = p_askWidget -> correctAnswers();
		QString q = p_askWidget -> getQuestionHook();
		QVector<userAnswer> ua = p_askWidget -> userAnswers();

		answersDialog *ad = new answersDialog(this, ua, q, ca);
		ad -> exec();
		delete ad;
	}
}

#include "kgeography.moc"
