/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreatureScript.h"
#include "ScriptedCreature.h"
#include "molten_core.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "Player.h"
#include "ThreatMgr.h"
#include "Creature.h"

enum Spells
{
    SPELL_IMPENDING_DOOM = 19702,
    SPELL_LUCIFRON_CURSE = 19703,
    SPELL_SHADOW_SHOCK = 20603,
    SPELL_LINK_PLAYERS = 102003,
    SPELL_LINK_PLAYERS_DAMAGE = 102004,
    SPELL_LINK_PLAYERS_VISUAL = 102005,
};

enum Events
{
    EVENT_IMPENDING_DOOM = 1,
    EVENT_LUCIFRON_CURSE = 2,
    EVENT_SHADOW_SHOCK = 3,
    EVENT_LINK_PLAYERS = 4,
    EVENT_LINK_PLAYERS_DAMAGE = 5,
    EVENT_LINK_PLAYERS_DONE = 6,
};

class boss_lucifron : public CreatureScript
{
public:
    boss_lucifron() : CreatureScript("boss_lucifron") { }

    struct boss_lucifronAI : public BossAI
    {
        boss_lucifronAI(Creature* creature) : BossAI(creature, DATA_LUCIFRON) {}

        Player* target_link1, * target_link2;

        void JustEngagedWith(Unit* /*who*/) override
        {
            _JustEngagedWith();
            if (me->GetMap()->IsHeroic()) {
                events.ScheduleEvent(EVENT_IMPENDING_DOOM, 5s, 10s);
                events.ScheduleEvent(EVENT_LUCIFRON_CURSE, 10s, 13s);
                events.ScheduleEvent(EVENT_SHADOW_SHOCK, 4s);
                events.ScheduleEvent(EVENT_LINK_PLAYERS, 10s);
            }
            else {
                events.ScheduleEvent(EVENT_IMPENDING_DOOM, 6s, 11s);
                events.ScheduleEvent(EVENT_LUCIFRON_CURSE, 11s, 14s);
                events.ScheduleEvent(EVENT_SHADOW_SHOCK, 5s);
            }

        }

        void ExecuteEvent(uint32 eventId) override
        {

            switch (eventId)
            {
            case EVENT_IMPENDING_DOOM:
            {
                DoCastVictim(SPELL_IMPENDING_DOOM);
                me->GetMap()->IsHeroic() ? events.RepeatEvent(18000) : events.RepeatEvent(20000);
                break;
            }
            case EVENT_LUCIFRON_CURSE:
            {
                DoCastVictim(SPELL_LUCIFRON_CURSE);
                me->GetMap()->IsHeroic() ? events.RepeatEvent(18000) : events.RepeatEvent(20000);
                break;
            }
            case EVENT_SHADOW_SHOCK:
            {
                DoCastVictim(SPELL_SHADOW_SHOCK);
                me->GetMap()->IsHeroic() ? events.RepeatEvent(4000) : events.RepeatEvent(5000);
                break;
            }
            case EVENT_LINK_PLAYERS:
            {
                std::list<Player*> playersInRange;
                Map* map = me->GetMap();

                if (map)
                {
                    Map::PlayerList const& PlayerList = map->GetPlayers();
                    for (auto i = PlayerList.begin(); i != PlayerList.end(); ++i)
                    {
                        if (Player* player = i->GetSource())
                        {
                            if (player->IsAlive() && me->GetDistance(player) <= 100.0f)
                            {
                                playersInRange.push_back(player);
                            }
                        }
                    }
                }

                if (Player* tank = me->GetThreatMgr().GetCurrentVictim()->ToPlayer()) {
                    playersInRange.remove(tank);
                }
                if (playersInRange.size() >= 2)
                {
                    std::list<Player*>::iterator it = playersInRange.begin();
                    std::advance(it, urand(0, playersInRange.size() - 1));
                    target_link1 = *it;
                    me->CastSpell(target_link1, SPELL_LINK_PLAYERS, false);

                    std::list<Player*>::iterator it2 = playersInRange.begin();
                    std::advance(it2, urand(0, playersInRange.size() - 1));
                    target_link2 = *it2;
                    me->CastSpell(target_link2, SPELL_LINK_PLAYERS, false);


                }
                events.RepeatEvent(30000);
                events.ScheduleEvent(EVENT_LINK_PLAYERS_DAMAGE, 0);
                events.ScheduleEvent(EVENT_LINK_PLAYERS_DONE, 15s);
                break;
            }
            case EVENT_LINK_PLAYERS_DAMAGE:
            {
                if (target_link1->IsAlive() && target_link2->IsAlive()) {
                    float distance = target_link1->GetDistance(target_link2->GetPosition());
                    if (distance > 10.0f) {
                        if (!target_link1->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) me->AddAura(SPELL_LINK_PLAYERS_DAMAGE, target_link1);
                        if (!target_link2->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) me->AddAura(SPELL_LINK_PLAYERS_DAMAGE, target_link2);
                        target_link1->CastSpell(target_link2, SPELL_LINK_PLAYERS_VISUAL, true);
                        target_link2->CastSpell(target_link1, SPELL_LINK_PLAYERS_VISUAL, true);
                    }
                    else {
                        if (target_link1->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) target_link1->RemoveAura(SPELL_LINK_PLAYERS_DAMAGE);
                        if (target_link2->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) target_link2->RemoveAura(SPELL_LINK_PLAYERS_DAMAGE);
                    }
                }
                events.RepeatEvent(100);
                break;
            }
            case EVENT_LINK_PLAYERS_DONE:
            {
                if (target_link1->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) target_link1->RemoveAura(SPELL_LINK_PLAYERS_DAMAGE);
                if (target_link2->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) target_link2->RemoveAura(SPELL_LINK_PLAYERS_DAMAGE);
                if (target_link1->HasAura(SPELL_LINK_PLAYERS)) target_link1->RemoveAura(SPELL_LINK_PLAYERS);
                if (target_link2->HasAura(SPELL_LINK_PLAYERS)) target_link2->RemoveAura(SPELL_LINK_PLAYERS);
                events.CancelEvent(EVENT_LINK_PLAYERS_DAMAGE);
            }
            }


        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetMoltenCoreAI<boss_lucifronAI>(creature);
    }
};

void AddSC_boss_lucifron()
{
    new boss_lucifron();
}
