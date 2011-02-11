/**
    Copyright (C) 2009,2010,2011  Dawn - 2D roleplaying game

    This file is a part of the dawn-rpg project <http://sourceforge.net/projects/dawn-rpg/>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. **/

#ifndef __STATS_H__
#define __STATS_H__

namespace StatsType
{
    enum StatsType
    {
        Dexterity,
        Intellect,
        Strength,
        Vitality,
        Wisdom,
        Health,
        Mana,
        Fatigue,
        Armor,
        DamageModifier,
        HitModifier,
        EvadeModifier,
        ParryModifier,
        BlockModifier,
        MeleeCritical,
        SpellCritical,
        ResistAll,
        SpellEffectAll,
        HealthRegen,
        ManaRegen,
        FatigueRegen,
        Count
    };
}

#endif
