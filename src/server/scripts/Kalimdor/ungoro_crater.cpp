/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Ungoro Crater
SD%Complete: 100
SDComment: Support for Quest: 4245, 4491
SDCategory: Ungoro Crater
EndScriptData */

/* ContentData
npc_a-me
npc_ringo
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "ScriptedFollowerAI.h"

enum AmeData
{
    SAY_READY               = -1000517,
    SAY_AGGRO1              = -1000518,
    SAY_SEARCH              = -1000519,
    SAY_AGGRO2              = -1000520,
    SAY_AGGRO3              = -1000521,
    SAY_FINISH              = -1000522,

    SPELL_DEMORALIZINGSHOUT = 13730,

    QUEST_CHASING_AME       = 4245,
    ENTRY_TARLORD           = 6519,
    ENTRY_TARLORD1          = 6519,
    ENTRY_STOMPER           = 6513,
};

class npc_ame : public CreatureScript
{
public:
    npc_ame() : CreatureScript("npc_ame") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_CHASING_AME)
        {
            CAST_AI(npc_escortAI, (creature->AI()))->Start(false, false, player->GetGUID());
            DoScriptText(SAY_READY, creature, player);
            creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
            // Change faction so mobs attack
            creature->setFaction(113);
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ameAI(creature);
    }

    struct npc_ameAI : public npc_escortAI
    {
        npc_ameAI(Creature* creature) : npc_escortAI(creature) {}

        uint32 DemoralizingShoutTimer;

        void WaypointReached(uint32 waypointId)
        {
            if (Player* player = GetPlayerForEscort())
            {
                switch (waypointId)
                {
                    case 19:
                        me->SummonCreature(ENTRY_STOMPER, -6391.69f, -1730.49f, -272.83f, 4.96f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                        DoScriptText(SAY_AGGRO1, me, player);
                        break;
                    case 28:
                        DoScriptText(SAY_SEARCH, me, player);
                        break;
                    case 38:
                        me->SummonCreature(ENTRY_TARLORD, -6370.75f, -1382.84f, -270.51f, 6.06f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                        DoScriptText(SAY_AGGRO2, me, player);
                        break;
                    case 49:
                        me->SummonCreature(ENTRY_TARLORD1, -6324.44f, -1181.05f, -270.17f, 4.34f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                        DoScriptText(SAY_AGGRO3, me, player);
                        break;
                    case 55:
                        DoScriptText(SAY_FINISH, me, player);
                        player->GroupEventHappens(QUEST_CHASING_AME, me);
                        break;
                }
            }
        }

        void Reset()
        {
            DemoralizingShoutTimer = 5000;
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->AI()->AttackStart(me);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(QUEST_CHASING_AME);
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
            if (!UpdateVictim())
                return;

            if (DemoralizingShoutTimer <= diff)
            {
                DoCast(me->getVictim(), SPELL_DEMORALIZINGSHOUT);
                DemoralizingShoutTimer = 70000;
            } else DemoralizingShoutTimer -= diff;
        }
    };
};

/*####
# npc_ringo
####*/

enum Ringo
{
    SAY_RIN_START_1             = -1000416,
    SAY_RIN_START_2             = -1000417,

    SAY_FAINT_1                 = -1000418,
    SAY_FAINT_2                 = -1000419,
    SAY_FAINT_3                 = -1000420,
    SAY_FAINT_4                 = -1000421,

    SAY_WAKE_1                  = -1000422,
    SAY_WAKE_2                  = -1000423,
    SAY_WAKE_3                  = -1000424,
    SAY_WAKE_4                  = -1000425,

    SAY_RIN_END_1               = -1000426,
    SAY_SPR_END_2               = -1000427,
    SAY_RIN_END_3               = -1000428,
    EMOTE_RIN_END_4             = -1000429,
    EMOTE_RIN_END_5             = -1000430,
    SAY_RIN_END_6               = -1000431, // signed for 6784
    SAY_SPR_END_7               = -1000432,
    EMOTE_RIN_END_8             = -1000433,

    SPELL_REVIVE_RINGO          = 15591,
    QUEST_A_LITTLE_HELP         = 4491,
    NPC_SPRAGGLE                = 9997,
    FACTION_ESCORTEE            = 113
};

class npc_ringo : public CreatureScript
{
public:
    npc_ringo() : CreatureScript("npc_ringo") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_A_LITTLE_HELP)
        {
            if (npc_ringoAI* pRingoAI = CAST_AI(npc_ringo::npc_ringoAI, creature->AI()))
            {
                creature->SetStandState(UNIT_STAND_STATE_STAND);
                pRingoAI->StartFollow(player, FACTION_ESCORTEE, quest);
            }
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ringoAI(creature);
    }

    struct npc_ringoAI : public FollowerAI
    {
        npc_ringoAI(Creature* creature) : FollowerAI(creature) { }

        uint32 FaintTimer;
        uint32 EndEventProgress;
        uint32 EndEventTimer;

        uint64 SpraggleGUID;

        void Reset()
        {
            FaintTimer = urand(30000, 60000);
            EndEventProgress = 0;
            EndEventTimer = 1000;
            SpraggleGUID = 0;
        }

        void MoveInLineOfSight(Unit* who)
        {
            FollowerAI::MoveInLineOfSight(who);

            if (!me->getVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && who->GetEntry() == NPC_SPRAGGLE)
            {
                if (me->IsWithinDistInMap(who, INTERACTION_DISTANCE))
                {
                    if (Player* player = GetLeaderForFollower())
                    {
                        if (player->GetQuestStatus(QUEST_A_LITTLE_HELP) == QUEST_STATUS_INCOMPLETE)
                            player->GroupEventHappens(QUEST_A_LITTLE_HELP, me);
                    }

                    SpraggleGUID = who->GetGUID();
                    SetFollowComplete(true);
                }
            }
        }

        void SpellHit(Unit* /*pCaster*/, const SpellInfo* pSpell)
        {
            if (HasFollowState(STATE_FOLLOW_INPROGRESS | STATE_FOLLOW_PAUSED) && pSpell->Id == SPELL_REVIVE_RINGO)
                ClearFaint();
        }

        void SetFaint()
        {
            if (!HasFollowState(STATE_FOLLOW_POSTEVENT))
            {
                SetFollowPaused(true);

                DoScriptText(RAND(SAY_FAINT_1, SAY_FAINT_2, SAY_FAINT_3, SAY_FAINT_4), me);
            }

            //what does actually happen here? Emote? Aura?
            me->SetStandState(UNIT_STAND_STATE_SLEEP);
        }

        void ClearFaint()
        {
            me->SetStandState(UNIT_STAND_STATE_STAND);

            if (HasFollowState(STATE_FOLLOW_POSTEVENT))
                return;

            DoScriptText(RAND(SAY_WAKE_1, SAY_WAKE_2, SAY_WAKE_3, SAY_WAKE_4), me);

            SetFollowPaused(false);
        }

        void UpdateFollowerAI(const uint32 Diff)
        {
            if (!UpdateVictim())
            {
                if (HasFollowState(STATE_FOLLOW_POSTEVENT))
                {
                    if (EndEventTimer <= Diff)
                    {
                        Unit* pSpraggle = Unit::GetUnit(*me, SpraggleGUID);
                        if (!pSpraggle || !pSpraggle->isAlive())
                        {
                            SetFollowComplete();
                            return;
                        }

                        switch (EndEventProgress)
                        {
                            case 1:
                                DoScriptText(SAY_RIN_END_1, me);
                                EndEventTimer = 3000;
                                break;
                            case 2:
                                DoScriptText(SAY_SPR_END_2, pSpraggle);
                                EndEventTimer = 5000;
                                break;
                            case 3:
                                DoScriptText(SAY_RIN_END_3, me);
                                EndEventTimer = 1000;
                                break;
                            case 4:
                                DoScriptText(EMOTE_RIN_END_4, me);
                                SetFaint();
                                EndEventTimer = 9000;
                                break;
                            case 5:
                                DoScriptText(EMOTE_RIN_END_5, me);
                                ClearFaint();
                                EndEventTimer = 1000;
                                break;
                            case 6:
                                DoScriptText(SAY_RIN_END_6, me);
                                EndEventTimer = 3000;
                                break;
                            case 7:
                                DoScriptText(SAY_SPR_END_7, pSpraggle);
                                EndEventTimer = 10000;
                                break;
                            case 8:
                                DoScriptText(EMOTE_RIN_END_8, me);
                                EndEventTimer = 5000;
                                break;
                            case 9:
                                SetFollowComplete();
                                break;
                        }

                        ++EndEventProgress;
                    }
                    else
                        EndEventTimer -= Diff;
                }
                else if (HasFollowState(STATE_FOLLOW_INPROGRESS) && !HasFollowState(STATE_FOLLOW_PAUSED))
                {
                    if (FaintTimer <= Diff)
                    {
                        SetFaint();
                        FaintTimer = urand(60000, 120000);
                    }
                    else
                        FaintTimer -= Diff;
                }

                return;
            }

            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_ungoro_crater()
{
    new npc_ame();
    new npc_ringo();
}
