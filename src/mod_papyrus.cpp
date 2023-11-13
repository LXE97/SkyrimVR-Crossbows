#include "mod_papyrus.h"

namespace Papyrus
{
    // AddItem
    typedef void (*_AddItem)(RE::BSTSmartPointer<RE::BSScript::IVirtualMachine> registry, uint32_t stackId, RE::TESObjectREFR *akSource, RE::TESForm *akItemToAdd, int aiCount, bool abSilent);
    RelocAddr<_AddItem> AddItem(0x009CDC20);

    taskAddItem::taskAddItem(RE::TESObjectREFR *akSource, RE::TESForm *akItemToAdd, int count)
    {
        m_akSource = akSource;
        m_akItemToAdd = akItemToAdd;
        m_count = count;
    }

    void taskAddItem::Run()
    {
        AddItem(RE::SkyrimVM::GetSingleton()->impl, 0, m_akSource, m_akItemToAdd, m_count, true);
    }

    void taskAddItem::Dispose() { delete this; }
}