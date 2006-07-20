/***************************************************************************
 *   Copyright (C) 2004-2005 by Albert Astals Cid                          *
 *   tsdgeos@terra.es                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <stdlib.h> // for RAND_MAX

#include <kacceleratormanager.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <krandom.h>

#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include "boxasker.h"
#include "map.h"

boxAsker::boxAsker(QWidget *parent, KGmap *m, QWidget *w, uint count) : askWidget(parent, m, w, count)
{
	p_lay = new QVBoxLayout(this);
	
	QGroupBox *bg = new QGroupBox(this);
	QVBoxLayout *gbLayout = new QVBoxLayout(bg);
	p_label = new QLabel(this);
	p_rb = new QRadioButton*[4];
	for(int i = 0; i < 4; i++)
	{
		p_rb[i] = new QRadioButton(bg);
		gbLayout -> addWidget(p_rb[i]);
	}
	gbLayout->addStretch(1);
	p_accept = new KPushButton(this);
	
	p_lay -> addWidget(p_label);
	p_lay -> addWidget(bg, 1);
	p_lay -> addWidget(p_accept);
	KAcceleratorManager::setNoAccel(this);
}

boxAsker::~boxAsker()
{
	delete[] p_rb;
}

void boxAsker::setQuestion(const QString &q)
{
	p_label -> setText(q);
}

void boxAsker::nextQuestionHook(const QString &division)
{
	QString otherDivision;
	QStringList auxList;
	int i;
	
	for(int i = 0; i < 4; i++) p_rb[i] -> setAutoExclusive(false);
	for(int i = 0; i < 4; i++) p_rb[i] -> setChecked(false);
	for(int i = 0; i < 4; i++) p_rb[i] -> setAutoExclusive(true);
	
	auxList << division;
		
	// we put the division in a random place
	p_position = (int)((float)4 * KRandom::random() / (RAND_MAX + 1.0));
	nextBoxAskerQuestionHook(division, p_position, true);
		
	// we put other 3 names
	i = 0;
	while (i < 4)
	{
		// false because boxaskers never are clickOnDivision
		otherDivision = p_map -> getRandomDivision(false);
		while (auxList.indexOf(otherDivision) != -1) otherDivision = p_map -> getRandomDivision(false);
		if (i == p_position) i++;
		if (i < 4 && nextBoxAskerQuestionHook(otherDivision, i, false)) i++;
		auxList << otherDivision;
	}
}

void boxAsker::checkAnswer()
{
	bool any, correct;
	int i;

	correct = false;	
	any = false;
	i = 0;
	while(!any && i < 4)
	{
		if (p_rb[i] -> isChecked())
		{
			any = true;
			correct = (i == p_position);
		}
		else i++;
	}
		
	if (any)
	{
		setAnswerHook(i);
		questionAnswered(correct);
		nextQuestion();
	}
}

void boxAsker::init()
{
	p_accept -> setText(i18n("&Accept"));

	resetAnswers();
	clearAsked();
	nextQuestion();
	
	p_accept -> disconnect();
	connect(p_accept, SIGNAL(clicked()), this, SLOT(checkAnswer()));
}

#include "boxasker.moc"
