#include "icqaccountmainsettings.h"
#include "ui_icqaccountmainsettings.h"
#include "icqaccount.h"
#include "icqprotocol_p.h"

namespace qutim_sdk_0_3 {

namespace oscar {

IcqAccountMainSettings::IcqAccountMainSettings(IcqAccount *account, QWidget *parent) :
    QWidget(parent),
	ui(new Ui::IcqAccountMainSettings),
	m_account(account)
{
    ui->setupUi(this);
	connect(ui->sslBox, SIGNAL(clicked(bool)), SLOT(onSslChecked(bool)));
	connect(ui->serverBox, SIGNAL(currentIndexChanged(QString)),
			SLOT(onCurrentServerChanged(QString)));
	connect(ui->serverBox, SIGNAL(editTextChanged(QString)),
			SIGNAL(completeChanged()));
	connect(ui->uinEdit, SIGNAL(textChanged(QString)),
			SIGNAL(completeChanged()));
	if (!account) {
		QRegExp rx("[1-9][0-9]{1,9}");
		QValidator *validator = new QRegExpValidator(rx, this);
		ui->uinEdit->setValidator(validator);
	}
	reloadSettings();
}

IcqAccountMainSettings::~IcqAccountMainSettings()
{
    delete ui;
}

void IcqAccountMainSettings::reloadSettings()
{
	ui->serverBox->clear();
	QString currentServer;
	if (m_account) {
		Config cfg = m_account->config();
		cfg.beginGroup("connection");
		bool ssl;
#ifdef OSCAR_SSL_SUPPORT
		ssl = cfg.value("ssl", false);
		ui->sslBox->setChecked(ssl);
#else
		ssl = false;
		ui->sslBox->setEnabled(false);
#endif
		currentServer = cfg.value("host", ssl ? QString("slogin.oscar.aol.com") : QString("login.icq.com"));
		ui->portBox->setValue(cfg.value("port", ssl ? 443 : 5190));
		ui->uinEdit->setText(m_account->id());
		ui->uinEdit->setEnabled(false);
		if (m_account->name() != m_account->id())
			ui->userNameEdit->setText(m_account->name());
		else
			ui->userNameEdit->clear();
		cfg.endGroup();
		cfg.beginGroup("general");
		ui->passwordEdit->setText(cfg.value("passwd", QString(), Config::Crypted));
		cfg.endGroup();
	} else {
		ui->uinEdit->clear();
		ui->userNameEdit->clear();
		ui->portBox->setValue(5190);
		ui->sslBox->setChecked(false);
		ui->passwordEdit->clear();
	}

#ifdef OSCAR_SSL_SUPPORT
	QHash<QString, QString> sslServers = defaultSslServers();
#endif
	int currentIndex = -1;
	int i = 0;
	foreach (const QString &server, defaultServers()) {
		ui->serverBox->addItem(server);
		if (currentIndex == -1 && currentServer == server)
			currentIndex = i;
#ifdef OSCAR_SSL_SUPPORT
		if (sslServers.contains(server)) {
			QString sslServer = sslServers.value(server);
			ui->serverBox->addItem(sslServer);
			if (currentIndex == -1 && currentServer == sslServer)
				currentIndex = i;
		}
#endif
		++i;
	}
	ui->serverBox->setCurrentIndex(currentIndex == -1 ? 0 : currentIndex);
}

void IcqAccountMainSettings::saveSettings()
{
	bool createAccount = !m_account;
	if (createAccount) {
		QString uin = ui->uinEdit->text();
		if (uin.isEmpty())
			return;
		m_account = new IcqAccount(uin);
	}

	Config cfg = m_account->config();
	cfg.beginGroup("general");
	QString newPassword = ui->passwordEdit->text();
	if (!newPassword.isEmpty())
		cfg.setValue("passwd", newPassword, Config::Crypted);
	else
		cfg.remove("passwd");
	QString nick = ui->userNameEdit->text();
	if (!nick.isEmpty())
		cfg.setValue("nick", ui->userNameEdit->text());
	else
		cfg.remove("nick");
	cfg.endGroup();
	cfg.beginGroup("connection");
	cfg.setValue("ssl", ui->sslBox->isChecked());
	cfg.setValue("host", ui->serverBox->currentText());
	cfg.setValue("port", ui->portBox->value());
	cfg.endGroup();

	if (createAccount)
		IcqProtocol::instance()->addAccount(m_account);
}

bool IcqAccountMainSettings::isComplete()
{
	return !ui->uinEdit->text().isEmpty() && !ui->serverBox->currentText().isEmpty();
}

const QStringList &IcqAccountMainSettings::defaultServers() const
{
	static QStringList list = QStringList()
							  << "login.oscar.aol.com"
							  << "ibucp-vip-d.blue.aol.com"
							  << "ibucp-vip-m.blue.aol.com"
							  << "ibucp2-vip-m.blue.aol.com"
							  << "bucp-m08.blue.aol.com"
							  << "icq.mirabilis.com"
							  << "icqalpha.mirabilis.com"
							  << "icq1.mirabilis.com"
							  << "icq2.mirabilis.com"
							  << "icq3.mirabilis.com"
							  << "icq4.mirabilis.com"
							  << "icq5.mirabilis.com";
	return list;
}

const QHash<QString, QString> &IcqAccountMainSettings::defaultSslServers() const
{
	static QHash<QString, QString> list;
	if (list.isEmpty())
		list.insert("login.oscar.aol.com", "slogin.oscar.aol.com");
	return list;
}

void IcqAccountMainSettings::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void IcqAccountMainSettings::onSslChecked(bool checked)
{
	// Update server if it is possible.
	QString currentServer = ui->serverBox->currentText();
	QString newServer = checked ?
						defaultSslServers().value(currentServer) :
						defaultSslServers().key(currentServer);
	if (!newServer.isEmpty()) {
		int newIndex = ui->serverBox->findText(newServer);
		if (newIndex == -1)
			ui->serverBox->setEditText(newServer);
		else
			ui->serverBox->setCurrentIndex(newIndex);
	}

	updatePort(checked);
}

void IcqAccountMainSettings::onCurrentServerChanged(const QString &currentServer)
{
	if (!ui->sslBox->isChecked()) {
		if (defaultSslServers().values().contains(currentServer)) {
			ui->sslBox->setChecked(true);
			updatePort(true);
		}
	} else {
		if (defaultServers().contains(currentServer)) {
			ui->sslBox->setChecked(false);
			updatePort(false);
		}
	}
}

void IcqAccountMainSettings::updatePort(bool ssl)
{
	if (ssl) {
		if (ui->portBox->value() == 5190)
			ui->portBox->setValue(443);
	} else {
		if (ui->portBox->value() == 443)
			ui->portBox->setValue(5190);
	}
}

IcqAccountMainSettingsWindget::IcqAccountMainSettingsWindget() :
	m_widget(0), m_layout(new QVBoxLayout(this))
{

}

IcqAccountMainSettingsWindget::~IcqAccountMainSettingsWindget()
{
}

void IcqAccountMainSettingsWindget::loadImpl()
{
	IcqAccount *account = qobject_cast<IcqAccount*>(controller());
	if (account) {
		m_widget = new IcqAccountMainSettings(account, this);
		m_layout->addWidget(m_widget);
		listenChildrenStates();
	}
}

void IcqAccountMainSettingsWindget::cancelImpl()
{
	if (m_widget)
		m_widget->reloadSettings();
}

void IcqAccountMainSettingsWindget::saveImpl()
{
	if (m_widget) {
		m_widget->saveSettings();
		emit saved();
	}
}

} } // namespace qutim_sdk_0_3::oscar
