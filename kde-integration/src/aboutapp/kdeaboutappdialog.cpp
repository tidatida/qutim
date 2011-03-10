/****************************************************************************
 *
 *  This file is part of qutIM
 *
 *  Copyright (c) 2011 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *
 ***************************************************************************
 *                                                                         *
 *   This file is part of free software; you can redistribute it and/or    *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************
 ****************************************************************************/

#include "kdeaboutappdialog.h"
#include <qutim/servicemanager.h>
#include <qutim/icon.h>
#include <qutim/debug.h>
#include <qutim/menucontroller.h>
#include <qutim/plugin.h>
#include <QCoreApplication>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <QHash>

using namespace qutim_sdk_0_3;

KdeAboutAppDialog::KdeAboutAppDialog()
{
	if (MenuController *menu = ServiceManager::getByName<MenuController*>("ContactList")) {
		ActionGenerator *gen = new ActionGenerator(Icon(QLatin1String("qutim")),
		                                           QT_TRANSLATE_NOOP("Core", "About qutIM"),
		                                           this,
		                                           SLOT(showWidget()));
		gen->setPriority(1);
		gen->setType(512);
		menu->addAction(gen);
	}
	m_data = 0;
}

KdeAboutAppDialog::~KdeAboutAppDialog()
{
	if (m_data)
		delete m_data;
}

void KdeAboutAppDialog::showWidget()
{
	if (m_widget) {
		m_widget->show();
		m_widget->raise();
		return;
	}
	if (m_data)
		delete m_data;
	
	QList<PersonInfo> authors = PersonInfo::authors();
	const KAboutData *aboutData = KGlobal::activeComponent().aboutData();
	m_data = new KAboutData(*aboutData);
	// Hook for now, fix it later
	LocalizedString localizedName("TRANSLATORS", "Your names");
	LocalizedString localizedEmail("TRANSLATORS", "Your emails");
	QString names = localizedName.toString();
	QString emails = localizedEmail.toString();
	if (names != QLatin1String(localizedName.original())) {
		if (emails == QLatin1String(localizedEmail.original()))
			emails.clear();
		m_data->setTranslator(ki18n(names.toUtf8()), ki18n(emails.toUtf8()));
	}
	for (int i = 0; i < authors.size(); i++) {
		const PersonInfo &info = authors.at(i);
		m_data->addAuthor(ki18n(info.name().toString().toUtf8()),
		                  ki18n(info.task().toString().toUtf8()),
		                  info.email().toUtf8(),
		                  info.web().toUtf8(),
		                  info.ocsUsername().toUtf8());
	}
	m_widget = new KAboutApplicationDialog(m_data);
	m_widget->show();
}
