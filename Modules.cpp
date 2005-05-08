#include "main.h"
#include "Modules.h"
#include "znc.h"
#include "Utils.h"
#include "User.h"
#include "Nick.h"
#include "Chan.h"

/////////////////// Timer ///////////////////
CTimer::CTimer(CModule* pModule, unsigned int uInterval, unsigned int uCycles, const CString& sLabel, const CString& sDescription) : CCron() {
	SetName(sLabel);
	m_sDescription = sDescription;
	m_pModule = pModule;

	if (uCycles) {
		StartMaxCycles(uInterval, uCycles);
	} else {
		Start(uInterval);
	}
}

CTimer::~CTimer() {
	m_pModule->UnlinkTimer(this);
}

void CTimer::SetModule(CModule* p) { m_pModule = p; }
void CTimer::SetDescription(const CString& s) { m_sDescription = s; }
CModule* CTimer::GetModule() const { return m_pModule; }
const CString& CTimer::GetDescription() const { return m_sDescription; }
/////////////////// !Timer ///////////////////

CModule::CModule(void* pDLL, CUser* pUser, const CString& sModName) {
	m_pDLL = pDLL;
	m_pManager = pUser->GetManager();
	m_pUser = pUser;
	m_sModName = sModName;
}

CModule::~CModule() {
	while (m_vTimers.size()) {
		RemTimer(m_vTimers[0]->GetName());
	}
}

bool CModule::AddTimer(CTimer* pTimer) {
	if ((!pTimer) || (FindTimer(pTimer->GetName()))) {
		delete pTimer;
		return false;
	}

	m_pManager->AddCron(pTimer);
	m_vTimers.push_back(pTimer);
	return true;
}

bool CModule::RemTimer(const CString& sLabel) {
	for (unsigned int a = 0; a < m_vTimers.size(); a++) {
		CTimer* pTimer = m_vTimers[a];

		if (pTimer->GetName().CaseCmp(sLabel) == 0) {
			m_vTimers.erase(m_vTimers.begin() +a);
			m_pManager->DelCronByAddr(pTimer);
			return true;
		}
	}

	return false;
}

bool CModule::UnlinkTimer(CTimer* pTimer) {
	for (unsigned int a = 0; a < m_vTimers.size(); a++) {
		if (pTimer == m_vTimers[a]) {
			m_vTimers.erase(m_vTimers.begin() +a);
			return true;
		}
	}

	return false;
}

CTimer* CModule::FindTimer(const CString& sLabel) {
	for (unsigned int a = 0; a < m_vTimers.size(); a++) {
		CTimer* pTimer = m_vTimers[a];
		if (pTimer->GetName().CaseCmp(sLabel) == 0) {
			return pTimer;
		}
	}

	return NULL;
}

void CModule::ListTimers() {
	if (!m_vTimers.size()) {
		PutModule("You have no timers running.");
		return;
	}

	CTable Table;
	Table.AddColumn("Name");
	Table.AddColumn("Secs");
	Table.AddColumn("Cycles");
	Table.AddColumn("Description");

	for (unsigned int a = 0; a < m_vTimers.size(); a++) {
		CTimer* pTimer = (CTimer*) m_vTimers[a];
		unsigned int uCycles = pTimer->GetCyclesLeft();

		Table.AddRow();
		Table.SetCell("Name", pTimer->GetName());
		Table.SetCell("Secs", CString::ToString(pTimer->GetInterval()));
		Table.SetCell("Cycles", ((uCycles) ? CString::ToString(uCycles) : "INF"));
		Table.SetCell("Description", pTimer->GetDescription());
	}

	if (Table.size()) {
		unsigned int uTableIdx = 0;
		CString sLine;

		while (Table.GetLine(uTableIdx++, sLine)) {
			PutModule(sLine);
		}
	}
}

const CString& CModule::GetModName() { return m_sModName; }
CString CModule::GetModNick() { return ((m_pUser) ? m_pUser->GetStatusPrefix() : "*") + m_sModName; }

CString CModule::GetDescription() { return "Unknown"; }

bool CModule::OnLoad(const CString& sArgs) { return true; }
bool CModule::OnBoot() { return true; }
void CModule::OnUserAttached() {}
void CModule::OnUserDetached() {}
void CModule::OnIRCDisconnected() {}
void CModule::OnIRCConnected() {}

bool CModule::OnDCCUserSend(const CNick& RemoteNick, unsigned long uLongIP, unsigned short uPort, const CString& sFile, unsigned long uFileSize) { return false; }

void CModule::OnOp(const CNick& OpNick, const CNick& Nick, const CChan& Channel, bool bNoChange) {}
void CModule::OnDeop(const CNick& OpNick, const CNick& Nick, const CChan& Channel, bool bNoChange) {}
void CModule::OnVoice(const CNick& OpNick, const CNick& Nick, const CChan& Channel, bool bNoChange) {}
void CModule::OnDevoice(const CNick& OpNick, const CNick& Nick, const CChan& Channel, bool bNoChange) {}
void CModule::OnRawMode(const CNick& OpNick, const CChan& Channel, const CString& sModes, const CString& sArgs) {}

bool CModule::OnUserRaw(CString& sLine) { return false; }
bool CModule::OnRaw(CString& sLine) { return false; }

bool CModule::OnStatusCommand(const CString& sCommand) { return false; }
void CModule::OnModCommand(const CString& sCommand) {}
void CModule::OnModNotice(const CString& sMessage) {}
void CModule::OnModCTCP(const CString& sMessage) {}

void CModule::OnQuit(const CNick& Nick, const CString& sMessage, const vector<CChan*>& vChans) {}
void CModule::OnNick(const CNick& Nick, const CString& sNewNick, const vector<CChan*>& vChans) {}
void CModule::OnKick(const CNick& Nick, const CString& sKickedNick, const CChan& Channel, const CString& sMessage) {}
void CModule::OnJoin(const CNick& Nick, const CChan& Channel) {}
void CModule::OnPart(const CNick& Nick, const CChan& Channel) {}

bool CModule::OnUserCTCPReply(const CNick& Nick, CString& sMessage) { return false; }
bool CModule::OnCTCPReply(const CNick& Nick, CString& sMessage) { return false; }
bool CModule::OnUserCTCP(const CString& sTarget, CString& sMessage) { return false; }
bool CModule::OnPrivCTCP(const CNick& Nick, CString& sMessage) { return false; }
bool CModule::OnChanCTCP(const CNick& Nick, const CChan& Channel, CString& sMessage) { return false; }
bool CModule::OnUserMsg(const CString& sTarget, CString& sMessage) { return false; }
bool CModule::OnPrivMsg(const CNick& Nick, CString& sMessage) { return false; }
bool CModule::OnChanMsg(const CNick& Nick, const CChan& Channel, CString& sMessage) { return false; }
bool CModule::OnUserNotice(const CString& sTarget, CString& sMessage) { return false; }
bool CModule::OnPrivNotice(const CNick& Nick, CString& sMessage) { return false; }
bool CModule::OnChanNotice(const CNick& Nick, const CChan& Channel, CString& sMessage) { return false; }

void* CModule::GetDLL() { return m_pDLL; }
bool CModule::PutIRC(const CString& sLine) {
	return (m_pUser) ? m_pUser->PutIRC(sLine) : false;
}
bool CModule::PutUser(const CString& sLine) {
	return (m_pUser) ? m_pUser->PutUser(sLine) : false;
}
bool CModule::PutStatus(const CString& sLine) {
	return (m_pUser) ? m_pUser->PutStatus(sLine) : false;
}
bool CModule::PutModule(const CString& sLine, const CString& sIdent, const CString& sHost) {
	return (m_pUser) ? m_pUser->PutUser(":" + GetModNick() + "!" + sIdent + "@" + sHost + " PRIVMSG " + m_pUser->GetCurNick() + " :" + sLine) : false;
}
bool CModule::PutModNotice(const CString& sLine, const CString& sIdent, const CString& sHost) {
	return (m_pUser) ? m_pUser->PutUser(":" + GetModNick() + "!" + sIdent + "@" + sHost + " NOTICE " + m_pUser->GetCurNick() + " :" + sLine) : false;
}

CModules::CModules() {}
CModules::~CModules() {}

void CModules::UnloadAll() {
	while (size()) {
		CString sRetMsg;
		CString sModName = (*this)[0]->GetModName();
		UnloadModule(sModName, sRetMsg);
	}
}

void CModules::OnIRCConnected() {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnIRCConnected();
	}
}

bool CModules::OnLoad(const CString& sArgs) {
	for (unsigned int a = 0; a < size(); a++) {
		if (!(*this)[a]->OnLoad(sArgs)) {
			return false;
		}
	}

	return true;
}

bool CModules::OnBoot() {
	for (unsigned int a = 0; a < size(); a++) {
		if (!(*this)[a]->OnBoot()) {
			return false;
		}
	}

	return true;
}

void CModules::OnUserAttached() {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnUserAttached();
	}
}

void CModules::OnUserDetached() {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnUserDetached();
	}
}

void CModules::OnIRCDisconnected() {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnIRCDisconnected();
	}
}

bool CModules::OnDCCUserSend(const CNick& RemoteNick, unsigned long uLongIP, unsigned short uPort, const CString& sFile, unsigned long uFileSize) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnDCCUserSend(RemoteNick, uLongIP, uPort, sFile, uFileSize)) {
			return true;
		}
	}

	return false;
}

void CModules::OnOp(const CNick& OpNick, const CNick& Nick, const CChan& Channel, bool bNoChange) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnOp(OpNick, Nick, Channel, bNoChange);
	}
}

void CModules::OnDeop(const CNick& OpNick, const CNick& Nick, const CChan& Channel, bool bNoChange) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnDeop(OpNick, Nick, Channel, bNoChange);
	}
}

void CModules::OnVoice(const CNick& OpNick, const CNick& Nick, const CChan& Channel, bool bNoChange) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnVoice(OpNick, Nick, Channel, bNoChange);
	}
}

void CModules::OnDevoice(const CNick& OpNick, const CNick& Nick, const CChan& Channel, bool bNoChange) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnDevoice(OpNick, Nick, Channel, bNoChange);
	}
}

void CModules::OnRawMode(const CNick& OpNick, const CChan& Channel, const CString& sModes, const CString& sArgs) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnRawMode(OpNick, Channel, sModes, sArgs);
	}
}

bool CModules::OnRaw(CString& sLine) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnRaw(sLine)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnUserRaw(CString& sLine) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnUserRaw(sLine)) {
			return true;
		}
	}

	return false;
}

void CModules::OnQuit(const CNick& Nick, const CString& sMessage, const vector<CChan*>& vChans) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnQuit(Nick, sMessage, vChans);
	}
}

void CModules::OnNick(const CNick& Nick, const CString& sNewNick, const vector<CChan*>& vChans) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnNick(Nick, sNewNick, vChans);
	}
}

void CModules::OnKick(const CNick& Nick, const CString& sKickedNick, const CChan& Channel, const CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnKick(Nick, sKickedNick, Channel, sMessage);
	}
}

void CModules::OnJoin(const CNick& Nick, const CChan& Channel) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnJoin(Nick, Channel);
	}
}

void CModules::OnPart(const CNick& Nick, const CChan& Channel) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnPart(Nick, Channel);
	}
}

bool CModules::OnUserCTCP(const CString& sTarget, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnUserCTCP(sTarget, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnUserCTCPReply(const CNick& Nick, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnUserCTCPReply(Nick, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnCTCPReply(const CNick& Nick, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnCTCPReply(Nick, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnPrivCTCP(const CNick& Nick, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnPrivCTCP(Nick, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnChanCTCP(const CNick& Nick, const CChan& Channel, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnChanCTCP(Nick, Channel, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnUserMsg(const CString& sTarget, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnUserMsg(sTarget, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnPrivMsg(const CNick& Nick, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnPrivMsg(Nick, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnChanMsg(const CNick& Nick, const CChan& Channel, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnChanMsg(Nick, Channel, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnUserNotice(const CString& sTarget, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnUserNotice(sTarget, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnPrivNotice(const CNick& Nick, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnPrivNotice(Nick, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnChanNotice(const CNick& Nick, const CChan& Channel, CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnChanNotice(Nick, Channel, sMessage)) {
			return true;
		}
	}

	return false;
}

bool CModules::OnStatusCommand(const CString& sCommand) {
	for (unsigned int a = 0; a < size(); a++) {
		if ((*this)[a]->OnStatusCommand(sCommand)) {
			return true;
		}
	}

	return false;
}

void CModules::OnModCommand(const CString& sCommand) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnModCommand(sCommand);
	}
}

void CModules::OnModNotice(const CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnModNotice(sMessage);
	}
}

void CModules::OnModCTCP(const CString& sMessage) {
	for (unsigned int a = 0; a < size(); a++) {
		(*this)[a]->OnModCTCP(sMessage);
	}
}

CModule* CModules::FindModule(const CString& sModule) {
	for (unsigned int a = 0; a < size(); a++) {
		if (sModule.CaseCmp((*this)[a]->GetModName()) == 0) {
			return (*this)[a];
		}
	}

	return NULL;
}

bool CModules::LoadModule(const CString& sModule, const CString& sArgs, CUser* pUser, CString& sRetMsg) {
#ifndef _MODULES
	sRetMsg = "Unable to load module [" + sModule + "] module support was not enabled.";
	return false;
#else
	sRetMsg = "";

	if (!pUser) {
		sRetMsg = "Unable to load module [" + sModule + "] Internal Error 1.";
		return false;
	}

	for (unsigned int a = 0; a < sModule.length(); a++) {
		if (((sModule[a] < '0') || (sModule[a] > '9')) && ((sModule[a] < 'a') || (sModule[a] > 'z')) && ((sModule[a] < 'A') || (sModule[a] > 'Z')) && (sModule[a] != '_')) {
			sRetMsg = "Unable to load module [" + sModule + "] module names can only be letters, numbers, or underscores.";
			return false;
		}
	}

	if (FindModule(sModule) != NULL) {
		sRetMsg = "Module [" + sModule + "] already loaded.";
		return false;
	}

	CString sModPath = pUser->GetCurPath() + "/modules/" + sModule + ".so";

	if (!CFile::Exists(sModPath)) {
		DEBUG_ONLY(cout << "[" << sModPath << "] Not found..." << endl);
		sModPath = pUser->GetModPath() + "/" + sModule + ".so";

		if (!CFile::Exists(sModPath))
		{
			DEBUG_ONLY(cout << "[" << sModPath << "] Not found..." << endl);
			sModPath = _MODDIR_ + CString("/") + sModule + ".so";

			if (!CFile::Exists(sModPath))
			{
				DEBUG_ONLY(cout << "[" << sModPath << "] Not found... giving up!" << endl);
				sRetMsg = "Unable to find module [" + sModule + "]";
				return false;
			}
		}
	}

	void* p = dlopen((sModPath).c_str(), RTLD_LAZY);

	if (!p) {
		sRetMsg = "Unable to load module [" + sModule + "] [" + dlerror() + "]";
		return false;
	}

	typedef double (*fpp)();
	fpp Version = (fpp) dlsym(p, "GetVersion");

	if (!Version) {
		dlclose(p);
		sRetMsg = "Could not find Version() in module [" + sModule + "]";
		return false;
	}

	if (CModule::GetVersion() != Version()) {
		dlclose(p);
		sRetMsg = "Version mismatch, recompile this module.";
		throw CException(CException::EX_BadModVersion);
		return false;
	}

	typedef CModule* (*fp)(void*, CUser* pUser, const CString& sModName);
	fp Load = (fp) dlsym(p, "Load");

	if (!Load) {
		dlclose(p);
		sRetMsg = "Could not find Load() in module [" + sModule + "]";
		return false;
	}

	CModule* pModule = Load(p, pUser, sModule);
	push_back(pModule);

	if (!pModule->OnLoad(sArgs)) {
		UnloadModule(sModule, sRetMsg);
		sRetMsg = "Module [" + sModule + "] aborted.";
		return false;
	}

	sRetMsg = "Loaded module [" + sModule + "] [" + sModPath + "]";
	return true;
#endif // !_MODULES
}

bool CModules::UnloadModule(const CString& sModule, CString& sRetMsg) {
#ifndef _MODULES
	sRetMsg = "Unable to unload module [" + sModule + "] module support was not enabled.";
	return false;
#else
	CModule* pModule = FindModule(sModule);
	sRetMsg = "";

	if (!pModule) {
		sRetMsg = "Module [" + sModule + "] not loaded.";
		return false;
	}

	void* p = pModule->GetDLL();

	if (p) {
		typedef void (*fp)(CModule*);
		fp Unload = (fp)dlsym(p, "Unload");

		if (Unload) {
			Unload(pModule);

			for (iterator it = begin(); it != end(); it++) {
				if (*it == pModule) {
					erase(it);
					break;
				}
			}

			dlclose(p);
			sRetMsg = "Module [" + sModule + "] unloaded";

			return true;
		} else {
			sRetMsg = "Unable to unload module [" + sModule + "] could not find Unload()";
			return false;
		}
	}

	sRetMsg = "Unable to unload module [" + sModule + "]";
	return false;
#endif // !_MODULES
}

bool CModules::ReloadModule(const CString& sModule, const CString& sArgs, CUser* pUser, CString& sRetMsg) {
	sRetMsg = "";
	if (!UnloadModule(sModule, sRetMsg)) {
		return false;
	}

	if (!LoadModule(sModule, sArgs, pUser, sRetMsg)) {
		return false;
	}

	sRetMsg = "Reloaded module [" + sModule + "]";
	return true;
}

void CModules::GetAvailableMods(set<CModInfo>& ssMods, CZNC* pZNC) {
	ssMods.clear();

	unsigned int a = 0;
	CDir Dir;

	Dir.FillByWildcard(pZNC->GetModPath(), "*.so");
	for (a = 0; a < Dir.size(); a++) {
		CFile& File = *Dir[a];
		ssMods.insert(CModInfo(File.GetShortName(), File.GetLongName(), false));
	}

	Dir.FillByWildcard(_MODDIR_, "*.so");
	for (a = 0; a < Dir.size(); a++) {
		CFile& File = *Dir[a];
		ssMods.insert(CModInfo(File.GetShortName(), File.GetLongName(), true));
	}
}
