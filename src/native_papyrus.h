#pragma once
#include "SKSE/Impl/Stubs.h"
#include "Relocation.h"

namespace Papyrus {
    class taskAddItem : public SKSE::detail::TaskDelegate {
    public:
        virtual void Run();
        virtual void Dispose();

        taskAddItem(RE::TESObjectREFR* akSource, RE::TESForm* akItemToAdd, int count);
        RE::TESObjectREFR* m_akSource;
        RE::TESForm* m_akItemToAdd;
        int m_count;
    };
}