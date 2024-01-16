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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "molten_core.h"

enum Spells
{
    SPELL_GEHENNAS_CURSE        = 19716,
    SPELL_RAIN_OF_FIRE          = 19717,
    SPELL_SHADOW_BOLT_RANDOM    = 19728,
    SPELL_SHADOW_BOLT_VICTIM    = 19729,
    SPELL_SHADOW_CLASH          = 102006,
};

enum Events
{
    EVENT_GEHENNAS_CURSE        = 1,
    EVENT_RAIN_OF_FIRE          = 2,     
    EVENT_SHADOW_BOLT           = 3,
    EVENT_SHADOW_CLASH          = 4,
    EVENT_SHADOW_CLASH_DAMAGE   = 5,
};

class boss_gehennas : public CreatureScript
{
public:
    boss_gehennas() : CreatureScript("boss_gehennas") { }

    struct boss_gehennasAI : public BossAI
    {
        boss_gehennasAI(Creature* creature) : BossAI(creature, DATA_GEHENNAS) {}

        Unit* clash_target;

        void JustEngagedWith(Unit* /*attacker*/) override
        {
            _JustEngagedWith();
            events.ScheduleEvent(EVENT_GEHENNAS_CURSE, 6s, 9s);
            events.ScheduleEvent(EVENT_RAIN_OF_FIRE, 10s);
            events.ScheduleEvent(EVENT_SHADOW_BOLT, 3s, 5s);
        }

        void ExecuteEvent(uint32 eventId) override
        {
            switch (eventId)
            {
                case EVENT_GEHENNAS_CURSE:
                {
                    DoCastVictim(SPELL_GEHENNAS_CURSE);
                    events.RepeatEvent(urand(25000, 30000));
                    break;
                }
                case EVENT_RAIN_OF_FIRE:
                {
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true))
                    {
                        DoCast(target, SPELL_RAIN_OF_FIRE, true);
                    }
                    events.RepeatEvent(6000);
                    break;
                }
                case EVENT_SHADOW_BOLT:
                {
                    if (urand(0, 1))
                    {
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 1, 0.0f, true))
                        {
                            DoCast(target, SPELL_SHADOW_BOLT_RANDOM);
                        }
                        else
                        {
                            DoCastVictim(SPELL_SHADOW_BOLT_VICTIM);
                        }
                    }
                    else
                    {
                        DoCastVictim(SPELL_SHADOW_BOLT_VICTIM);
                    }

                    events.RepeatEvent(5000);
                    break;
                }
                case EVENT_SHADOW_CLASH:
                {
                    clash_target = SelectTarget(SelectTargetMethod::Random, 1, 0.0f, true);
                    events.ScheduleEvent(EVENT_SHADOW_CLASH_DAMAGE, 5000);
                    break;
                }
                case EVENT_SHADOW_CLASH_DAMAGE:
                {
                    break;
                }

            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetMoltenCoreAI<boss_gehennasAI>(creature);
    }
};

void AddSC_boss_gehennas()
{
    new boss_gehennas();
}
