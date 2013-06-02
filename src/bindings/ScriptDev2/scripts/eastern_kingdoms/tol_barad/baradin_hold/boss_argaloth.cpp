/* Copyright (C) 2006 - 2013 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: boss_argaloth
SD%Complete: 10
SDComment: Placeholder
SDCategory: Baradin Hold
EndScriptData */

#include "precompiled.h"
#include "baradin_hold.h"

enum Spells
{
    SPELL_FEL_FIRESTORM       = 88972,
    SPELL_CONSUMING_DARKNESS  = 88954,
    SPELL_METEOR_SLASH        = 88942,
    SPELL_BERSERK             = 47008,
};

enum ePhases
{
    PHASE_1     = 1,
    PHASE_2,
};

struct MANGOS_DLL_DECL boss_argalothAI : public ScriptedAI
{
    boss_argalothAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;
    uint8 Phase, PhaseCount;
    uint32 SlashTimer;
    uint32 ConsumingDarknessTimer;
    uint32 BerserkTimer;
    uint32 ResetPhaseTimer;

    void Reset() override
    {
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);

        PhaseCount = 0;
        SlashTimer = 10000;
        ConsumingDarknessTimer = 15000;
        BerserkTimer = 360000;
        Phase = PHASE_1;
    }

    void Aggro(Unit* pWho) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_ARGALOTH, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_ARGALOTH, DONE);
    }

    void KilledUnit(Unit* pVictim) override
    {
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_ARGALOTH, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (!UpdateVictim() || me->IsNonMeleeSpellCasted(false))
            return;

        if (SlashTimer <= uiDiff && Phase == PHASE_1)
        {
            DoCastVictim(SPELL_METEOR_SLASH);
            SlashTimer = 15000;
        }
        else
            SlashTimer -= uiDiff;

        if (ConsumingDarknessTimer <= uiDiff && Phase == PHASE_1)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCast(target, SPELL_CONSUMING_DARKNESS, true);

            ConsumingDarknessTimer = 15000;
        }
        else
            ConsumingDarknessTimer -= uiDiff;

        if (BerserkTimer <= uiDiff && Phase == PHASE_1)
        {
            DoCast(me, SPELL_BERSERK);
            BerserkTimer = 360000;
        }
        else
            BerserkTimer -= uiDiff;

        if (me->GetHealthPercent() < 67.0f && Phase == PHASE_1 && PhaseCount == 0)
        {
            ++PhaseCount;
            Phase = PHASE_2;
            me->AttackStop();
            DoCastAOE(SPELL_FEL_FIRESTORM);
            ResetPhaseTimer = 16500;
        }

        if (me->GetHealthPercent() < 34 && Phase == PHASE_1 && PhaseCount == 1)
        {
            ++PhaseCount;
            Phase = PHASE_2;
            me->AttackStop();
            DoCastAOE(SPELL_FEL_FIRESTORM);
            ResetPhaseTimer = 16500;
        }

        if (ResetPhaseTimer <= uiDiff)
        {
            Phase = PHASE_1;
            SlashTimer = 10000;
            ConsumingDarknessTimer = 15000;
            ResetPhaseTimer = 360000;
        }
        else
            ResetPhaseTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_argaloth(Creature* pCreature)
{
    return new boss_argalothAI(pCreature);
}

void AddSC_boss_argaloth()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_argaloth";
    pNewScript->GetAI = &GetAI_boss_argaloth;
    pNewScript->RegisterSelf();
}
