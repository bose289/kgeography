/***************************************************************************
 *   Copyright (C) 2004 by Albert Astals Cid                               *
 *   tsdgeos@terra.es                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KGEOGRAPHY_H
#define KGEOGRAPHY_H

#include <kmainwindow.h>

class KPushButton;
class KAction;

class QLabel;
class QWidgetStack;

class map;
class askWidget;

class kgeography : public KMainWindow
{
Q_OBJECT
	public:
		kgeography();
		~kgeography();
	
	private slots:
		void goToMenu();
	
		void openMap();
	
		void consult();
		void askCapitals();
		void askMap();
		void askFlags();
		
		void putMenu();
		
	private:
		void putAskWidget();
		void setMap(map *m);
		
		QWidgetStack *p_stack;
		
		KPushButton *p_consult;
		KPushButton *p_askCapitals;
		KPushButton *p_askMap;
		KPushButton *p_askFlags;
		KAction *p_goToMenu;
		QLabel *p_currentMap;
	
		map *p_map;
		askWidget *p_askWidget;
};

#endif
