/**
    Copyright (C) 2009,2010  Dawn - 2D roleplaying game

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

#include "CCharacter.h"
#include <cassert>
#include "CDrawingHelpers.h"
#include "CSpell.h"
#include "CInterface.h"
#include "GroundLoot.h"
#include "StatsSystem.h"
#include "CZone.h"

#include "CNPC.h"
#include "Player.h"
#include "globals.h"

#include <map>
#include <string>
#include <limits>
#include <iostream>
#include <memory>

std::map< std::string, CCharacter* > allMobTypes;

// Dawn LUA Interface
namespace DawnInterface
{
	CCharacter* createNewMobType( std::string typeID )
	{
		CCharacter *newMobType = new CNPC(0, 0, 0, 0, 0);
		allMobTypes[ typeID ] = newMobType;
		return newMobType;
	}
	void addTextToLogWindow( GLfloat color[], const char *text, ... );
}

void CCharacter::baseOnType( std::string otherName )
{
	CCharacter *other;
	if ( allMobTypes.count( otherName ) != 1 ) {
		std::cerr << "mob type \"" << otherName << "\" found " << allMobTypes.count( otherName ) << " times!!! aborting." << std::endl;
		abort();
	}
	other = allMobTypes[ otherName ];
	// classID is only set if this was created as a new mob type with createNewMobType
	if ( this->classID == "" ) {
		this->classID = otherName;
	}
	setStrength( other->getStrength() );
	setDexterity( other->getDexterity() );
	setVitality( other->getVitality() );
	setIntellect( other->getIntellect() );
	setWisdom( other->getWisdom() );
	setMaxHealth( other->getMaxHealth() );
	setMaxMana( other->getMaxMana() );
	setMaxFatigue( other->getMaxFatigue() );
	setMinDamage( other->getMinDamage() );
	setMaxDamage( other->getMaxDamage() );
	size_t numActivities=static_cast<size_t>(ActivityType::Count);
	for ( size_t curActivity=0; curActivity<numActivities; ++curActivity ) {
		ActivityType::ActivityType curActivityType = static_cast<ActivityType::ActivityType>( curActivity );
		setNumMoveTexturesPerDirection( curActivityType, other->numMoveTexturesPerDirection[ curActivity ] );
		setTexture( curActivityType, other->getTexture(curActivityType) );
	}
	setArmor( other->getArmor() );
	setHealthRegen( other->getHealthRegen() );
	setManaRegen( other->getManaRegen() );
	setFatigueRegen( other->getFatigueRegen() );
	setDamageModifierPoints( other->getDamageModifierPoints() );
	setHitModifierPoints( other->getHitModifierPoints() );
	setEvadeModifierPoints( other->getEvadeModifierPoints() );
	setParryModifierPoints( other->getParryModifierPoints() );
	setBlockModifierPoints( other->getBlockModifierPoints() );
	setMeleeCriticalModifierPoints( other->getMeleeCriticalModifierPoints() );
	setResistAllModifierPoints( other->getResistAllModifierPoints() );
	setSpellEffectAllModifierPoints( other->getSpellEffectAllModifierPoints() );
	for ( size_t curElement=0; curElement<static_cast<size_t>(ElementType::Count); ++curElement ) {
		ElementType::ElementType curElementType = static_cast<ElementType::ElementType>(curElement);
		setResistElementModifierPoints( curElementType, other->getResistElementModifierPoints( curElementType ) );
		setSpellEffectElementModifierPoints( curElementType, other->getSpellEffectElementModifierPoints( curElementType ) );
	}
	setSpellCriticalModifierPoints( other->getSpellCriticalModifierPoints() );
	setClass( other->getClass() );
	setWanderRadius ( other->getWanderRadius() );
	setName( other->getName() );
	setLevel( other->getLevel() );
	setLootTable( other->getLootTable() );
	setBoundingBox( other->getBoundingBoxX(), other->getBoundingBoxY(), other->getBoundingBoxW(), other->getBoundingBoxH() );
	setUseBoundingBox( other->getUseBoundingBox() );
	setCoinDrop( other->minCoinDrop, other->maxCoinDrop, other->coinDropChance );
	setSpellbook( other->getSpellbook() );
}

std::string CCharacter::getClassID() const
{
	return classID;
}

const uint16_t NULLABLE_ATTRIBUTE_MIN = 0;
const uint16_t NON_NULLABLE_ATTRIBUTE_MIN = 1;

template <class AttributeType, class ModifierType>
AttributeType getModifiedAttributeValue( AttributeType attributeValue, ModifierType modifier,
                                         AttributeType minValue = std::numeric_limits<AttributeType>::min(),
                                         AttributeType maxValue = std::numeric_limits<AttributeType>::max() )
{
	assert( attributeValue >= minValue && attributeValue <= maxValue );
	// is modified value < minValue? => set to minValue
	if ( modifier < 0 && static_cast<AttributeType>(-modifier) > attributeValue - minValue )
		return minValue;
	// is modified value > maxValue? => set to maxValue
	else if ( modifier > 0 && (maxValue - attributeValue) < modifier )
		return maxValue;
	else
		return (attributeValue + modifier);
}

CCharacter* CCharacter::getTarget() const
{
	return target;
}

void CCharacter::setTarget( CCharacter *target )
{
    this->target = target;
}

std::string CCharacter::getName() const
{
	return name;
}

void CCharacter::setName( std::string newName )
{
	name = newName;
}

void CCharacter::setArmor( uint16_t newArmor )
{
    armor = newArmor;
}

uint16_t CCharacter::getArmor() const
{
    return armor;
}

uint16_t CCharacter::getModifiedArmor() const
{
	return getArmor() + StatsSystem::getStatsSystem()->calculateDamageReductionPoints( this );
}

void CCharacter::modifyArmor( int16_t armorModifier )
{
    setArmor( getModifiedAttributeValue( armor, armorModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setDamageModifierPoints( uint16_t newDamageModifierPoints )
{
	damageModifierPoints = newDamageModifierPoints;
}

uint16_t CCharacter::getModifiedDamageModifierPoints() const
{
	return getDamageModifierPoints() + StatsSystem::getStatsSystem()->calculateDamageModifierPoints( this );
}

uint16_t CCharacter::getDamageModifierPoints() const
{
	return damageModifierPoints;
}

void CCharacter::modifyDamageModifierPoints( int16_t damageModifierPointsModifier )
{
	setDamageModifierPoints( getModifiedAttributeValue( damageModifierPoints, damageModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setHitModifierPoints( uint16_t newHitModifierPoints )
{
	hitModifierPoints = newHitModifierPoints;
}

uint16_t CCharacter::getModifiedHitModifierPoints() const
{
	return getHitModifierPoints() + StatsSystem::getStatsSystem()->calculateHitModifierPoints( this );
}

uint16_t CCharacter::getHitModifierPoints() const
{
	return hitModifierPoints;
}

void CCharacter::modifyHitModifierPoints( int16_t hitModifierPointsModifier )
{
	setHitModifierPoints( getModifiedAttributeValue( hitModifierPoints, hitModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setEvadeModifierPoints( uint16_t newEvadeModifierPoints )
{
	evadeModifierPoints = newEvadeModifierPoints;
}

uint16_t CCharacter::getModifiedEvadeModifierPoints() const
{
	return getEvadeModifierPoints() + StatsSystem::getStatsSystem()->calculateEvadeModifierPoints( this );
}

uint16_t CCharacter::getEvadeModifierPoints() const
{
	return evadeModifierPoints;
}

void CCharacter::modifyEvadeModifierPoints( int16_t evadeModifierPointsModifier )
{
	setEvadeModifierPoints( getModifiedAttributeValue( evadeModifierPoints, evadeModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setParryModifierPoints( uint16_t newParryModifierPoints )
{
	parryModifierPoints = newParryModifierPoints;
}

uint16_t CCharacter::getModifiedParryModifierPoints() const
{
	return getParryModifierPoints() + StatsSystem::getStatsSystem()->calculateParryModifierPoints( this );
}

uint16_t CCharacter::getParryModifierPoints() const
{
	return parryModifierPoints;
}

void CCharacter::modifyParryModifierPoints( int16_t parryModifierPointsModifier )
{
	setParryModifierPoints( getModifiedAttributeValue( parryModifierPoints, parryModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setBlockModifierPoints( uint16_t newBlockModifierPoints )
{
	blockModifierPoints = newBlockModifierPoints;
}

uint16_t CCharacter::getModifiedBlockModifierPoints() const
{
	return getBlockModifierPoints() + StatsSystem::getStatsSystem()->calculateBlockModifierPoints( this );
}

uint16_t CCharacter::getBlockModifierPoints() const
{
	return blockModifierPoints;
}

void CCharacter::modifyBlockModifierPoints( int16_t blockModifierPointsModifier )
{
	setBlockModifierPoints( getModifiedAttributeValue( blockModifierPoints, blockModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setMeleeCriticalModifierPoints( uint16_t newMeleeCriticalModifierPoints )
{
	meleeCriticalModifierPoints = newMeleeCriticalModifierPoints;
}

uint16_t CCharacter::getModifiedMeleeCriticalModifierPoints() const
{
	return getMeleeCriticalModifierPoints() + StatsSystem::getStatsSystem()->calculateMeleeCriticalModifierPoints( this );
}

uint16_t CCharacter::getMeleeCriticalModifierPoints() const
{
	return meleeCriticalModifierPoints;
}

void CCharacter::modifyMeleeCriticalModifierPoints( int16_t meleeCriticalModifierPointsModifier )
{
	setMeleeCriticalModifierPoints( getModifiedAttributeValue( meleeCriticalModifierPoints, meleeCriticalModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setResistElementModifierPoints( ElementType::ElementType elementType, uint16_t newResistElementModifierPoints )
{
	resistElementModifierPoints[ static_cast<size_t>(elementType) ] = newResistElementModifierPoints;
}

uint16_t CCharacter::getModifiedResistElementModifierPoints( ElementType::ElementType elementType ) const
{
	return getResistElementModifierPoints( elementType ) + getResistAllModifierPoints() +  StatsSystem::getStatsSystem()->calculateResistElementModifierPoints( elementType, this );
}

uint16_t CCharacter::getResistElementModifierPoints( ElementType::ElementType elementType ) const
{
	return resistElementModifierPoints[ static_cast<size_t>(elementType) ];
}

void CCharacter::modifyResistElementModifierPoints( ElementType::ElementType elementType, int16_t resistElementModifierPointsModifier )
{
	setResistElementModifierPoints( elementType, getModifiedAttributeValue( resistElementModifierPoints[ static_cast<size_t>(elementType) ], resistElementModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setResistAllModifierPoints( uint16_t newResistAllModifierPoints )
{
	resistAllModifierPoints = newResistAllModifierPoints;
}

uint16_t CCharacter::getResistAllModifierPoints() const
{
	return resistAllModifierPoints;
}

void CCharacter::modifyResistAllModifierPoints( int16_t resistAllModifierPointsModifier )
{
	setResistAllModifierPoints( getModifiedAttributeValue( resistAllModifierPoints, resistAllModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setSpellEffectElementModifierPoints( ElementType::ElementType elementType, uint16_t newSpellEffectElementModifierPoints )
{
	spellEffectElementModifierPoints[ static_cast<size_t>(elementType) ] = newSpellEffectElementModifierPoints;
}

uint16_t CCharacter::getModifiedSpellEffectElementModifierPoints( ElementType::ElementType elementType ) const
{
	return getSpellEffectElementModifierPoints( elementType ) + getSpellEffectAllModifierPoints() +  StatsSystem::getStatsSystem()->calculateSpellEffectElementModifierPoints( elementType, this );
}

uint16_t CCharacter::getSpellEffectElementModifierPoints( ElementType::ElementType elementType ) const
{
	return spellEffectElementModifierPoints[ static_cast<size_t>(elementType) ];
}

void CCharacter::modifySpellEffectElementModifierPoints( ElementType::ElementType elementType, int16_t spellEffectElementModifierPointsModifier )
{
	setSpellEffectElementModifierPoints( elementType, getModifiedAttributeValue( spellEffectElementModifierPoints[ static_cast<size_t>(elementType) ], spellEffectElementModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setSpellEffectAllModifierPoints( uint16_t newSpellEffectAllModifierPoints )
{
	spellEffectAllModifierPoints = newSpellEffectAllModifierPoints;
}

uint16_t CCharacter::getSpellEffectAllModifierPoints() const
{
	return spellEffectAllModifierPoints;
}

void CCharacter::modifySpellEffectAllModifierPoints( int16_t spellEffectAllModifierPointsModifier )
{
	setSpellEffectAllModifierPoints( getModifiedAttributeValue( spellEffectAllModifierPoints, spellEffectAllModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setSpellCriticalModifierPoints( uint16_t newSpellCriticalModifierPoints )
{
	spellCriticalModifierPoints = newSpellCriticalModifierPoints;
}

uint16_t CCharacter::getModifiedSpellCriticalModifierPoints() const
{
	return getSpellCriticalModifierPoints() + StatsSystem::getStatsSystem()->calculateSpellCriticalModifierPoints( this );
}

uint16_t CCharacter::getSpellCriticalModifierPoints() const
{
	return spellCriticalModifierPoints;
}

void CCharacter::modifySpellCriticalModifierPoints( int16_t spellCriticalModifierPointsModifier )
{
	setSpellCriticalModifierPoints( getModifiedAttributeValue( spellCriticalModifierPoints, spellCriticalModifierPointsModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

uint16_t CCharacter::getStrength() const
{
	return strength;
}

uint16_t CCharacter::getModifiedStrength() const
{
	return getStrength();
}

void CCharacter::setStrength( uint16_t newStrength )
{
	assert( newStrength >= NON_NULLABLE_ATTRIBUTE_MIN );
	strength = newStrength;
}

void CCharacter::modifyStrength( int16_t strengthModifier )
{
	setStrength( getModifiedAttributeValue( strength, strengthModifier, NON_NULLABLE_ATTRIBUTE_MIN ) );
}

uint16_t CCharacter::getDexterity() const
{
	return dexterity;
}

uint16_t CCharacter::getModifiedDexterity() const
{
	return getDexterity();
}

void CCharacter::setDexterity( uint16_t newDexterity )
{
	assert( newDexterity >= NON_NULLABLE_ATTRIBUTE_MIN );
	dexterity = newDexterity;
}

void CCharacter::modifyDexterity( int16_t dexterityModifier )
{
	setDexterity( getModifiedAttributeValue( dexterity, dexterityModifier, NON_NULLABLE_ATTRIBUTE_MIN ) );
}

uint16_t CCharacter::getVitality() const
{
	return vitality;
}

uint16_t CCharacter::getModifiedVitality() const
{
	return getVitality();
}

void CCharacter::setVitality( uint16_t newVitality )
{
	assert( newVitality >= NON_NULLABLE_ATTRIBUTE_MIN );
	vitality = newVitality;
}

void CCharacter::modifyVitality( int16_t vitalityModifier )
{
	setVitality( getModifiedAttributeValue( vitality, vitalityModifier, NON_NULLABLE_ATTRIBUTE_MIN ) );
}

uint16_t CCharacter::getIntellect() const
{
	return intellect;
}

uint16_t CCharacter::getModifiedIntellect() const
{
	return getIntellect();
}

void CCharacter::setIntellect( uint16_t newIntellect )
{
	assert( newIntellect >= NON_NULLABLE_ATTRIBUTE_MIN );
	intellect = newIntellect;
}

void CCharacter::modifyIntellect( int16_t intellectModifier )
{
	setIntellect( getModifiedAttributeValue( intellect, intellectModifier, NON_NULLABLE_ATTRIBUTE_MIN ) );
}

uint16_t CCharacter::getWisdom() const
{
	return wisdom;
}

uint16_t CCharacter::getModifiedWisdom() const
{
	return getWisdom();
}

void CCharacter::setWisdom( uint16_t newWisdom )
{
	assert( newWisdom >= NON_NULLABLE_ATTRIBUTE_MIN );
	wisdom = newWisdom;
}

void CCharacter::modifyWisdom( int16_t wisdomModifier )
{
	setWisdom( getModifiedAttributeValue( wisdom, wisdomModifier, NON_NULLABLE_ATTRIBUTE_MIN ) );
}

uint16_t CCharacter::getMaxHealth() const
{
	return max_health;
}

uint16_t CCharacter::getModifiedMaxHealth() const
{
	return getMaxHealth();
}

void CCharacter::setMaxHealth( uint16_t newMaxHealth )
{
	assert( newMaxHealth >= NON_NULLABLE_ATTRIBUTE_MIN );
	max_health = newMaxHealth;
	// if ( current_health > getModifiedMaxHealth() )
	// {
		current_health = getModifiedMaxHealth();
	// }
}

void CCharacter::modifyMaxHealth( int16_t maxHealthModifier )
{
	setMaxHealth( getModifiedAttributeValue( max_health, maxHealthModifier, NON_NULLABLE_ATTRIBUTE_MIN ) );
}

uint16_t CCharacter::getCurrentHealth() const
{
	if ( current_health > getModifiedMaxHealth() )
		return getModifiedMaxHealth();

	return current_health;
}

void CCharacter::setCurrentHealth( uint16_t newCurrentHealth )
{
	assert( newCurrentHealth <= getModifiedMaxHealth() );
	current_health = newCurrentHealth;
}

void CCharacter::modifyCurrentHealth( int16_t currentHealthModifier )
{
	setCurrentHealth( getModifiedAttributeValue( getCurrentHealth(), currentHealthModifier, NULLABLE_ATTRIBUTE_MIN, getModifiedMaxHealth() ) );
}

uint16_t CCharacter::getMaxMana() const
{
	return max_mana;
}

uint16_t CCharacter::getModifiedMaxMana() const
{
	return getMaxMana();
}

void CCharacter::setMaxMana( uint16_t newMaxMana )
{
	max_mana = newMaxMana;
	// if ( current_mana > getModifiedMaxMana() )
	// {
		current_mana = getModifiedMaxMana();
	// }
}

void CCharacter::modifyMaxMana( int16_t maxManaModifier )
{
	setMaxMana( getModifiedAttributeValue( max_mana, maxManaModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

uint16_t CCharacter::getCurrentMana() const
{
	if ( current_mana > getModifiedMaxMana() )
		return getModifiedMaxMana();

	return current_mana;
}

void CCharacter::setCurrentMana( uint16_t newCurrentMana )
{
	assert( newCurrentMana <= getModifiedMaxMana() );
	current_mana = newCurrentMana;
}

void CCharacter::modifyCurrentMana( int16_t currentManaModifier )
{
	setCurrentMana( getModifiedAttributeValue( getCurrentMana(), currentManaModifier, NULLABLE_ATTRIBUTE_MIN, getModifiedMaxMana() ) );
}


uint16_t CCharacter::getMaxFatigue() const
{
	return max_fatigue;
}

uint16_t CCharacter::getModifiedMaxFatigue() const
{
	return getMaxFatigue();
}

void CCharacter::setMaxFatigue( uint16_t newMaxFatigue )
{
	max_fatigue = newMaxFatigue;
	// if ( current_fatigue > getModifiedMaxFatigue() )
	// {
		current_fatigue = getModifiedMaxFatigue();
	// }
}

void CCharacter::modifyMaxFatigue( int16_t maxFatigueModifier )
{
	setMaxFatigue( getModifiedAttributeValue( max_fatigue, maxFatigueModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

uint16_t CCharacter::getCurrentFatigue() const
{
	if ( current_fatigue > getModifiedMaxFatigue() )
		return getModifiedMaxFatigue();

	return current_fatigue;
}

void CCharacter::setCurrentFatigue( uint16_t newCurrentFatigue )
{
	assert( newCurrentFatigue <= getModifiedMaxFatigue() );
	current_fatigue = newCurrentFatigue;
}

void CCharacter::modifyCurrentFatigue( int16_t currentFatigueModifier )
{
	setCurrentFatigue( getModifiedAttributeValue( getCurrentFatigue(), currentFatigueModifier, NULLABLE_ATTRIBUTE_MIN, getModifiedMaxFatigue() ) );
}

void CCharacter::setManaRegen( uint16_t newManaRegen )
{
    assert( newManaRegen >= NULLABLE_ATTRIBUTE_MIN );
	manaRegen = newManaRegen;
}

uint16_t CCharacter::getModifiedManaRegen() const
{
    return getManaRegen();
}

uint16_t CCharacter::getManaRegen() const
{
    return manaRegen;
}

void CCharacter::modifyManaRegen( int16_t manaRegenModifier )
{
    setManaRegen( getModifiedAttributeValue( manaRegen, manaRegenModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setFatigueRegen( uint16_t newFatigueRegen )
{
    assert( newFatigueRegen >= NULLABLE_ATTRIBUTE_MIN );
	fatigueRegen = newFatigueRegen;
}

uint16_t CCharacter::getModifiedFatigueRegen() const
{
    return getFatigueRegen();
}

uint16_t CCharacter::getFatigueRegen() const
{
    return fatigueRegen;
}

void CCharacter::modifyFatigueRegen( int16_t fatigueRegenModifier )
{
    setFatigueRegen( getModifiedAttributeValue( fatigueRegen, fatigueRegenModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setHealthRegen( uint16_t newHealthRegen )
{
    assert( newHealthRegen >= NULLABLE_ATTRIBUTE_MIN );
	healthRegen = newHealthRegen;
}

uint16_t CCharacter::getModifiedHealthRegen() const
{
    return getHealthRegen();
}

uint16_t CCharacter::getHealthRegen() const
{
    return healthRegen;
}

void CCharacter::modifyHealthRegen( int16_t healthRegenModifier )
{
    setHealthRegen( getModifiedAttributeValue( healthRegen, healthRegenModifier, NULLABLE_ATTRIBUTE_MIN ) );
}

void CCharacter::setMinDamage( uint16_t newMinDamage )
{
	min_damage = newMinDamage;
}

uint16_t CCharacter::getMinDamage() const
{
	return min_damage;
}

uint16_t CCharacter::getModifiedMinDamage() const
{
	return getMinDamage();
}

void CCharacter::setMaxDamage( uint16_t newMaxDamage )
{
	max_damage = newMaxDamage;
}

uint16_t CCharacter::getMaxDamage() const
{
	return max_damage;
}

uint16_t CCharacter::getModifiedMaxDamage() const
{
	return getMaxDamage();
}

void CCharacter::addItemToLootTable(Item *item, double dropChance )
{
    lootTable.push_back( sLootTable( item, dropChance ) );
}

uint64_t CCharacter::getExperience() const
{
	return experience;
}

void CCharacter::gainExperience( uint64_t addExp )
{
	if ( isPlayer() ) {

        if ( std::numeric_limits<uint64_t>::max() - addExp < experience ) {
            experience = std::numeric_limits<uint64_t>::max();
            dawn_debug_warn( "max experience reached" );
        } else {
            experience += addExp;
            GLfloat yellow[] = { 1.0f, 1.0f, 0.0f };
            DawnInterface::addTextToLogWindow( yellow, "You gain %d experience.", addExp );
        }

        while ( canRaiseLevel() ) {
            raiseLevel();
        }
	}
}

uint64_t CCharacter::getExpNeededForLevel( uint8_t level ) const
{
	assert( level > 0 );
	uint64_t result = (level*(level-1)* 50);
	return result;
}

bool CCharacter::canRaiseLevel() const
{
	return ( experience >= getExpNeededForLevel( getLevel() + 1 ) && ( getExpNeededForLevel( getLevel() + 1 ) != getExpNeededForLevel( getLevel() ) ) );
}

void CCharacter::raiseLevel()
{
	if ( canRaiseLevel() ) {
		setMaxHealth( getMaxHealth() * 1.1 );
		setStrength( getStrength() * 1.1 );
		setLevel( getLevel() + 1 );
		GLfloat yellow[] = { 1.0f, 1.0f, 0.0f };
		if ( isPlayer() == true ) {
		    dynamic_cast<Player*>(this)->setTicketForItemTooltip();
		    dynamic_cast<Player*>(this)->setTicketForSpellTooltip();
		}
		DawnInterface::addTextToLogWindow( yellow, "You are now a level %d %s.", getLevel(), getClassName().c_str() );
	}
}

void CCharacter::setClass( CharacterClass::CharacterClass characterClass )
{
    this->characterClass = characterClass;
    switch ( characterClass )
    {
        /// all caster classes here...
        case CharacterClass::Liche:
            characterArchType = CharacterArchType::Caster;
        break;

        /// and all other fighter classes here...
        case CharacterClass::Ranger:
        case CharacterClass::Warrior:
            characterArchType = CharacterArchType::Fighter;
        break;
    }
}

CharacterClass::CharacterClass CCharacter::getClass() const
{
    return characterClass;
}

CharacterArchType::CharacterArchType CCharacter::getArchType() const
{
    return characterArchType;
}

std::string CCharacter::getClassName() const
{
    return CharacterClass::getCharacterClassName( getClass() );
}

void CCharacter::setWanderRadius( uint16_t newWanderRadius )
{
	wander_radius = newWanderRadius;
}

uint16_t CCharacter::getWanderRadius() const
{
	return wander_radius;
}

void CCharacter::setLevel ( uint8_t newLevel )
{
	assert( newLevel > 0 );
	level = newLevel;
}

uint8_t CCharacter::getLevel() const
{
	assert( level > 0 );
	return level;
}

void CCharacter::setLootTable( std::vector<sLootTable> newLootTable )
{
    lootTable = newLootTable;
}

std::vector<sLootTable> CCharacter::getLootTable() const
{
    return lootTable;
}

void CCharacter::setTexture( ActivityType::ActivityType activity, CTexture *newTexture )
{
	size_t activityNr = static_cast<size_t>( activity );
	this->texture[ activityNr ] = newTexture;
}

CTexture* CCharacter::getTexture( ActivityType::ActivityType activity ) const
{
	size_t activityNr = static_cast<size_t>( activity );
	return this->texture[ activityNr ];
}

void CCharacter::setNumMoveTexturesPerDirection( ActivityType::ActivityType activity, int numTextures )
{
	size_t activityNr = static_cast<size_t>(activity);
	numMoveTexturesPerDirection[ activityNr ] = numTextures;
	assert( texture[ activityNr ] == NULL );

	texture[ activityNr ] = new CTexture();
	texture[ activityNr ]->texture.resize( 8 * numTextures + 1 );
}

void CCharacter::setMoveTexture( ActivityType::ActivityType activity, int direction, int index, std::string filename )
{
	size_t activityNr = static_cast<size_t>(activity);
	assert( texture[activityNr] != NULL );
	assert( index < numMoveTexturesPerDirection[activityNr] );

	texture[ activityNr ]->LoadIMG( filename, direction + 8*index );
}

// end of Dawn LUA Interface

CCharacter::CCharacter()
	: strength( 1 ),
	  dexterity( 1 ),
	  vitality( 1 ),
	  intellect( 1 ),
	  wisdom( 1 ),
	  max_health( 1 ),
	  current_health( 1 ),
	  max_mana( 0 ),
	  current_mana( 0 ),
      healthRegen( 0 ),
	  manaRegen( 0 ),
	  armor( 0 ),
	  damageModifierPoints( 0 ),
	  hitModifierPoints( 0 ),
	  evadeModifierPoints( 0 ),
	  blockModifierPoints( 0 ),
	  meleeCriticalModifierPoints( 0 ),
	  resistElementModifierPoints( NULL ),
	  resistAllModifierPoints( 0 ),
	  spellEffectElementModifierPoints( NULL ),
	  spellEffectAllModifierPoints( 0 ),
	  spellCriticalModifierPoints( 0 ),
	  level( 1 ),
	  boundingBoxX( 0 ),
	  boundingBoxY( 0 ),
	  boundingBoxW( 0 ),
	  boundingBoxH( 0 ),
	  useBoundingBox( false ),
	  minCoinDrop( 0 ),
	  maxCoinDrop( 0 ),
	  coinDropChance( 0.0 )
{
	resistElementModifierPoints = new uint16_t[ static_cast<size_t>( ElementType::Count ) ];
	spellEffectElementModifierPoints = new uint16_t[ static_cast<size_t>( ElementType::Count ) ];
	for ( size_t curElement=0; curElement<static_cast<size_t>( ElementType::Count ); ++curElement ) {
		resistElementModifierPoints[ curElement ] = 0;
		spellEffectElementModifierPoints[ curElement ] = 0;
	}
	size_t numActivities = static_cast<size_t>( ActivityType::Count );
	numMoveTexturesPerDirection = new int[ numActivities ];
	texture = new CTexture* [ numActivities ];
	for ( size_t curActivity=0; curActivity<numActivities; ++curActivity ) {
		numMoveTexturesPerDirection[ curActivity ] = 0;
		texture[ curActivity ] = NULL;
	}
	activeDirection = S;
}

CCharacter::~CCharacter()
{
	if ( isPreparing ) {
		// note: if the current spell / action is not null this means
		//       it is still bound to the player. He alone is responsible
		//       for it.
		delete curSpellAction;
	}
	size_t numActivities = static_cast<size_t>( ActivityType::Count );
	for ( size_t curActivity=0; curActivity<numActivities; ++curActivity ) {
		if ( texture[ curActivity ] != NULL ) {
			delete texture[ curActivity ];
		}
	}
	delete[] numMoveTexturesPerDirection;
	delete[] texture;
}

int CCharacter::getXPos() const
{
	return x_pos;
}

int CCharacter::getYPos() const
{
	return y_pos;
}

int CCharacter::getWidth() const
{
	return useBoundingBox ? boundingBoxW : texture[0]->texture[1].width;
}

int CCharacter::getHeight() const
{
	return useBoundingBox ? boundingBoxH : texture[0]->texture[1].height;
}

void CCharacter::setPosition( int xpos, int ypos )
{
	this->x_pos = xpos;
	this->y_pos = ypos;
}

extern std::vector <CNPC*> NPC;

extern Player character;

bool hasIntersection( int r1_l, int r1_r, int r1_b, int r1_t, int r2_l, int r2_r, int r2_b, int r2_t )
{
	return ( ! ( (r1_t < r2_b) || (r1_b > r2_t ) || (r1_l > r2_r) || (r1_r < r2_l) ) );
}

int CCharacter::CheckForCollision(int x_pos, int y_pos)
{
	CZone *curZone = Globals::getCurrentZone();

	int character_l = x_pos, character_r = x_pos + getWidth(), character_b = y_pos, character_t = y_pos + getHeight();
	for (unsigned int t=0;t<curZone->CollisionMap.size();t++) {
		int other_l = curZone->CollisionMap[t].CR.x, other_r = curZone->CollisionMap[t].CR.x+curZone->CollisionMap[t].CR.w;
		int other_b = curZone->CollisionMap[t].CR.y, other_t = curZone->CollisionMap[t].CR.y+curZone->CollisionMap[t].CR.h;
		if ( hasIntersection( other_l, other_r, other_b, other_t,
		                      character_l, character_r, character_b, character_t )) {
			return 1;
		}
	}

	// check for collision with other characters
	std::vector<CNPC*> zoneNPCs = curZone->getNPCs();
	for ( size_t curNPCNr=0; curNPCNr < zoneNPCs.size(); ++curNPCNr )
	{
		CCharacter *curNPC = zoneNPCs[ curNPCNr ];
		if ( curNPC == this || ! curNPC->isAlive() )
			continue;

		int other_l = curNPC->getXPos(), other_r = curNPC->getXPos() + curNPC->getWidth();
		int other_b = curNPC->getYPos(), other_t = curNPC->getYPos() + curNPC->getHeight();

		if ( hasIntersection( other_l, other_r, other_b, other_t,
		                      character_l, character_r, character_b, character_t ) ) {
				return 1;
		}

	}

	// check for collision with player
	{
		CCharacter *curNPC = &character;
		if ( curNPC != this && curNPC->isAlive() ) {
			int other_l = curNPC->getXPos(), other_r = curNPC->getXPos() + curNPC->getWidth();
			int other_b = curNPC->getYPos(), other_t = curNPC->getYPos() + curNPC->getHeight();

			if ( hasIntersection( other_l, other_r, other_b, other_t,
			                      character_l, character_r, character_b, character_t ) ) {
				return 1;
			}
		}
	}

	return 0;
}

int CCharacter::CollisionCheck(Direction direction)
{

	switch (direction) {
		case N:
			// check upper left corner
			if (CheckForCollision(x_pos,y_pos+1) == 1) {
				return 1;
			}
		break;

		case E:
			// check upper right corner
			if (CheckForCollision(x_pos+1,y_pos) == 1) {
				return 1;
			}
		break;

		case S:
			// check lower left corner
			if (CheckForCollision(x_pos,y_pos-1) == 1) {
				return 1;
			}
		break;

		case W:
			// check upper left corner
			if (CheckForCollision(x_pos-1,y_pos) == 1) {
				return 1;
			}
		break;

		default:
		break;
	}
	return 0;
}

void CCharacter::MoveUp()
{
	if (CollisionCheck(N) == 0) {
		y_pos++;
	}
}

void CCharacter::MoveDown()
{
	if (CollisionCheck(S) == 0) {
		y_pos--;
	}
}

void CCharacter::MoveLeft()
{
	if (CollisionCheck(W) == 0) {
		x_pos--;
	}
}

void CCharacter::MoveRight()
{
	if (CollisionCheck(E) == 0) {
		x_pos++;
	}
}

void CCharacter::Move()
{
    if ( isStunned() == true ) {
        remainingMovePoints = 0;
        return;
    }

	continuePreparing();
	if ( ! mayDoAnythingAffectingSpellActionWithoutAborting() ) {
		if ( ! mayDoAnythingAffectingSpellActionWithAborting() ) {
			remainingMovePoints = 0;
			return;
		}
	}

	Direction movingDirection = GetDirection();
	if (( movingDirection != STOP) && ! mayDoAnythingAffectingSpellActionWithoutAborting() ) {
		CastingAborted();
	}

	unsigned int movePerStep = 10; // moves one step per movePerStep ms

	// To balance moving diagonally boost, movePerStep = 10*sqrt(2)
	if ( movingDirection == NW || movingDirection == NE || movingDirection == SW || movingDirection == SE )
		movePerStep = 14;

    // recalculate the movementpoints based on our movementspeed (spells that affect this can be immobolizing spells, snares or movement enhancer
    remainingMovePoints *= getMovementSpeed();

	while ( remainingMovePoints > movePerStep ) {
		remainingMovePoints -= movePerStep;
		switch ( movingDirection ) {
			case NW:
				MoveLeft();
				MoveUp();
				break;
			case N:
				MoveUp();
				break;
			case NE:
				MoveRight();
				MoveUp();
				break;
			case W:
				MoveLeft();
				break;
			case E:
				MoveRight();
				break;
			case SW:
				MoveLeft();
				MoveDown();
				break;
			case S:
				MoveDown();
				break;
			case SE:
				MoveRight();
				MoveDown();
				break;
			default:
				break;
		}
	}
}

void CCharacter::giveMovePoints( uint32_t movePoints )
{
	remainingMovePoints += movePoints;
}

Direction CCharacter::getDirectionTowards( int x_pos, int y_pos ) const
{
	int dx = x_pos - (this->x_pos + this->getWidth()) / 2;
	int dy = y_pos - (this->y_pos + this->getHeight()) / 2;

	if ( dx > 0 ) {
		if ( dy > 0 ) {
			return NE;
		} else if ( dy < 0 ) {
			return SE;
		} else {
			return E;
		}
	} else if ( dx < 0 ) {
		if ( dy > 0 ) {
			return NW;
		} else if ( dy < 0 ) {
			return SW;
		} else {
			return W;
		}
	} else {
		if ( dy > 0 ) {
			return N;
		} else if ( dy < 0 ) {
			return S;
		} else {
			return STOP;
		}
	}
}

ActivityType::ActivityType CCharacter::getCurActivity() const
{
	ActivityType::ActivityType curActivity = ActivityType::Walking;
	if ( curSpellAction != NULL ) {
		if ( dynamic_cast<CSpell*>( curSpellAction ) != NULL ) {
			curActivity = ActivityType::Casting;
		} else if ( dynamic_cast<RangedDamageAction*>( curSpellAction ) != NULL ) {
		    curActivity = ActivityType::Shooting;
        } else if ( dynamic_cast<MeleeDamageAction*>( curSpellAction ) != NULL ) {
			curActivity = ActivityType::Attacking;
		}
	}
	return curActivity;
}

int CCharacter::GetDirectionTexture()
{
	if ( isStunned() == true ) {
	    return activeDirection;
	}

	int direction = GetDirection();
	if ( direction != STOP ) {
		activeDirection = direction;
	}
	ActivityType::ActivityType curActivity = getCurActivity();

	switch ( curActivity ) {
		case ActivityType::Walking:
		{
			if ( direction == STOP )
				return activeDirection;

			int msPerDrawFrame = 100;
			int index = ((SDL_GetTicks() % (msPerDrawFrame * numMoveTexturesPerDirection[ curActivity ] )) / msPerDrawFrame );
			return static_cast<int>(direction) + 8*index;
		}
		break;
		case ActivityType::Casting:
		case ActivityType::Attacking:
		case ActivityType::Shooting:
		{
			double progress;
			if ( curActivity == ActivityType::Casting ) {
				progress = getPreparationPercentage();
			} else if ( curActivity == ActivityType::Shooting ) {
			    progress = getPreparationPercentage(); // we handle shooting just like spells, since aiming and the like would be a preperation stage.
			} else {
				progress = dynamic_cast<CAction*>(curSpellAction)->getProgress();
			}
			int index = progress * numMoveTexturesPerDirection[ curActivity ];
			if ( index >= numMoveTexturesPerDirection[ curActivity ] ) {
				index = numMoveTexturesPerDirection[ curActivity ] - 1;
			}
			return static_cast<int>(activeDirection) + 8*index;
		}
		default:
			assert( false );
			return 0;
		break;
	}
}

void CCharacter::executeSpellWithoutCasting( CSpellActionBase *spell, CCharacter *target )
{
    assert ( spell != NULL );
    assert ( target != NULL );
    CSpellActionBase *newSpell = NULL;

    newSpell = spell->cast( this, target );

    if ( newSpell != NULL ) {
        newSpell->startEffect();
    }
}

// use this function to cast spells with rules (mana requirement, range etc...
void CCharacter::castSpell( CSpellActionBase *spell )
{
    if ( isStunned() == true ) {
        /// can't cast, we're stunned. Should perhaps display message about it.
        return;
    }

    if ( dynamic_cast<CAction*>( spell ) != NULL ) {
        if ( spell->getSpellCost() > getCurrentFatigue() ) {
            /// can't cast. cost more fatigue than we can afford. Display message here about it.
            return;
        }
	} else if ( dynamic_cast<CSpell*>( spell) != NULL ) {
	    if ( spell->getSpellCost() > getCurrentMana() )	{
	        /// can't cast. not enough mana. Display message here about it.
            return;
	    }
	}

	if ( spell->getEffectType() != EffectType::SelfAffectingSpell && getTarget() != NULL ) {
	    uint16_t distance = sqrt( pow( ( getXPos() + getWidth() / 2 ) - ( getTarget()->getXPos() + getTarget()->getWidth() / 2 ),2) + pow( ( getYPos() + getHeight() / 2 ) - ( getTarget()->getYPos() + getTarget()->getHeight() / 2 ),2) );
        if ( spell->isInRange( distance ) == false ) {
            /// can't cast, not in range. Display message here about it...
            return;
        }
	}

	for (size_t curSpell = 0; curSpell < cooldownSpells.size(); curSpell++)
	{
	    if ( cooldownSpells[curSpell].first->getID() == spell->getID() )
	    {
	        if ( SDL_GetTicks() < cooldownSpells[curSpell].second + spell->getCooldown() * 1000 )
            {
                /// can't cast, spell has a cooldown on it. Display message about it.
                return;
            }
	    }
	}

    giveToPreparation( spell );
}

void CCharacter::giveToPreparation( CSpellActionBase *toPrepare )
{
	if ( curSpellAction != NULL ) {
		// don't cast / execute. Enqueue in the list of coming actions / spells ?
		delete toPrepare;
	} else {
		// setup all variables for casting / executing
		isPreparing = true;
		curSpellAction = toPrepare;
		toPrepare->beginPreparationOfSpellAction();
		preparationStartTime = SDL_GetTicks();
		continuePreparing();
	}
}

bool CCharacter::continuePreparing()
{
    /// if we're preparing a spell while getting stunned, abort the spellcasting.
	if ( isStunned() == true && getIsPreparing() == true ) {
	    CastingAborted();
	}

	if ( isPreparing ) {
		bool preparationFinished = (curSpellAction->getCastTime() == 0);
		if ( ! preparationFinished ) {
			preparationCurrentTime = SDL_GetTicks();

			// casting_percentage is mostly just for the castbar display, guess we could alter this code.
			preparationPercentage = (static_cast<float>(preparationCurrentTime-preparationStartTime)) / curSpellAction->getCastTime();
			preparationFinished = ( preparationPercentage >= 1.0f );
		}
		if ( preparationFinished ) {
			startSpellAction();
		}
	}

	return isPreparing;
}

void CCharacter::startSpellAction()
{
	isPreparing = false;
	preparationCurrentTime = 0;
	preparationStartTime = 0;

	curSpellAction->startEffect();
}

void CCharacter::abortCurrentSpellAction()
{
	assert( curSpellAction != NULL );
	if ( isPreparing ) {
		delete curSpellAction;
		curSpellAction = NULL;
		isPreparing = false;
	}
}

bool CCharacter::getIsPreparing() const
{
    return isPreparing;
}

std::string CCharacter::getCurrentSpellActionName() const
{
    if ( curSpellAction != NULL ) {
        return curSpellAction->getName();
    }
    return "";
}

void CCharacter::CastingAborted()
{
	// if we moved, got stunned, or in some way unable to complete the spell ritual, spellcasting will fail.
	// If we are following the above instructions to use a pointer to a spell and so on, we should clear that pointer here.
	abortCurrentSpellAction();
}

void CCharacter::CastingInterrupted()
{
	// when casting a spell, mobs attacking us in any way should interfere with our spellcasting, slowing us down a bit.
	// so if we have a spell with 5 seconds spellcast, and we're up at 4 seconds of casting.. getting hit at that moment
	// should set the current_castingtime back to say 3.2 or so..
	preparationStartTime += 500; // for now using a static pushback of the spellcasting, 0.5 seconds.
	if (preparationStartTime > preparationCurrentTime) {
		preparationStartTime = preparationCurrentTime;
	}
}

float CCharacter::getPreparationPercentage() const
{
	if ( isPreparing ) {
		return preparationPercentage;
	} else {
		return 0;
	}
}

bool CCharacter::mayDoAnythingAffectingSpellActionWithoutAborting() const
{
	return ( curSpellAction == NULL );
}

bool CCharacter::mayDoAnythingAffectingSpellActionWithAborting() const
{
	if ( isPlayer() == true ) {
	    return ( curSpellAction == NULL || isPreparing );
	} else {
	    return ( curSpellAction == NULL );
	}
}

void CCharacter::Damage(int amount, bool criticalHit)
{
	if (alive) {
        addDamageDisplayToGUI( amount, criticalHit, 0 );
		if (current_health <= amount) {
			current_health = 0;
			Die();
		} else {
			modifyCurrentHealth( -amount );
		}
	}
}

void CCharacter::regenerateLifeManaFatigue(uint32_t regenPoints)
{
    /** Regenerate life, mana and fatigue every 1000 ms. **/

	remainingRegenPoints += regenPoints;

	if ( remainingRegenPoints > 1000 ) {
		modifyCurrentMana( getModifiedManaRegen() );
		modifyCurrentHealth( getModifiedHealthRegen() );
		modifyCurrentFatigue( getModifiedFatigueRegen() );
		remainingRegenPoints -= 1000;
	}
}

extern size_t randomSizeT( size_t min, size_t max );

void CCharacter::dropItems()
{
    // iterate through the loot table and see if we should drop any items.
    CZone *curZone = Globals::getCurrentZone();

    for ( size_t tableID = 0; tableID < lootTable.size(); ++tableID )
    {
        double dropChance = (double)rand()/(double)RAND_MAX;
        if ( dropChance <= lootTable[tableID].dropChance )
        {
            curZone->getGroundLoot()->addItem( getXPos(), getYPos(), lootTable[tableID].item );
        }
    }

    {
    	double dropChance = (double)rand()/(double)RAND_MAX;
    	if ( dropChance <= coinDropChance ) {
    		curZone->getGroundLoot()->addItem( getXPos(), getYPos(), new GoldHeap( randomSizeT( minCoinDrop, maxCoinDrop ) ) );
    	}
    }
}

void CCharacter::Die()
{


}

void CCharacter::Heal(int amount)
{
	if (alive) {
		uint16_t modifiedDiff = getModifiedMaxHealth() - getCurrentHealth();
	    if ( modifiedDiff <= amount ) {
			amount = modifiedDiff;
		}
		addDamageDisplayToGUI( amount, false, 1 );
		modifyCurrentHealth( amount );
	}
}

void CCharacter::setActiveGUI( CInterface *GUI_ )
{
    activeGUI = GUI_;
}

void CCharacter::addDamageDisplayToGUI( int amount, bool critical, uint8_t damageType )
{
    if (isPlayer())
	{
        activeGUI->addCombatText(amount, critical, damageType, world_x+140,dawn_configuration::screenHeight-40+world_y);
	} else {
	    activeGUI->addCombatText(amount, critical, damageType, getXPos() + getWidth()/2, getYPos()+getHeight()+52);
	}
}

bool CCharacter::isStunned() const
{
    for ( size_t activeSpell = 0; activeSpell < activeSpells.size(); activeSpell++ ) {
        if ( activeSpells[ activeSpell ].first->getEffect().first == CharacterStates::Stunned ) {
            return true;
        }
    }
    return false;
}

bool CCharacter::isCharmed() const
{
    for ( size_t activeSpell = 0; activeSpell < activeSpells.size(); activeSpell++ ) {
        if ( activeSpells[ activeSpell ].first->getEffect().first == CharacterStates::Charmed ) {
            return true;
        }
    }
    return false;
}

bool CCharacter::isFeared() const
{
    for ( size_t activeSpell = 0; activeSpell < activeSpells.size(); activeSpell++ ) {
        if ( activeSpells[ activeSpell ].first->getEffect().first == CharacterStates::Feared ) {
            return true;
        }
    }
    return false;
}

bool CCharacter::isInvisible() const
{
    for ( size_t activeSpell = 0; activeSpell < activeSpells.size(); activeSpell++ ) {
        if ( activeSpells[ activeSpell ].first->getEffect().first == CharacterStates::Invisible ) {
            return true;
        }
    }
    return false;
}

bool CCharacter::isSneaking() const
{
    for ( size_t activeSpell = 0; activeSpell < activeSpells.size(); activeSpell++ ) {
        if ( activeSpells[ activeSpell ].first->getEffect().first == CharacterStates::Sneaking ) {
            return true;
        }
    }
    return false;
}

bool CCharacter::isConfused() const
{
    for ( size_t activeSpell = 0; activeSpell < activeSpells.size(); activeSpell++ ) {
        if ( activeSpells[ activeSpell ].first->getEffect().first == CharacterStates::Confused ) {
            return true;
        }
    }
    return false;
}

float CCharacter::getMovementSpeed() const
{
    // see if we are affected by movement altering spells. If we are we get the lowest value and return it.
    // if we have no spell lowering the movement, we look for enhancers and return that. If that's not found, 1.0 is returned.
    float lowestMovementSpeed = 1.0f;
    float highestMovementSpeed = 1.0f;
    for ( size_t activeSpell = 0; activeSpell < activeSpells.size(); activeSpell++ ) {
        if ( activeSpells[ activeSpell ].first->getEffect().first == CharacterStates::Movementspeed ) {
            if ( lowestMovementSpeed > activeSpells[ activeSpell ].first->getEffect().second ) {
                lowestMovementSpeed = activeSpells[ activeSpell ].first->getEffect().second;
            }
            if ( highestMovementSpeed < activeSpells[ activeSpell ].first->getEffect().second ) {
                highestMovementSpeed = activeSpells[ activeSpell ].first->getEffect().second;
            }
        }
    }
    if ( lowestMovementSpeed < 1.0 ) {
        return lowestMovementSpeed;
    } else {
        return highestMovementSpeed;
    }
}

void CCharacter::setBoundingBox( int bbx, int bby, int bbw, int bbh )
{
	boundingBoxX = bbx;
	boundingBoxY = bby;
	boundingBoxW = bbw;
	boundingBoxH = bbh;
}

void CCharacter::setUseBoundingBox( bool use )
{
	useBoundingBox = use;
}

int CCharacter::getBoundingBoxX() const
{
	return boundingBoxX;
}

int CCharacter::getBoundingBoxY() const
{
	return boundingBoxY;
}

int CCharacter::getBoundingBoxW() const
{
	return boundingBoxW;
}

int CCharacter::getBoundingBoxH() const
{
	return boundingBoxH;
}

bool CCharacter::getUseBoundingBox() const
{
	return useBoundingBox;
}

void CCharacter::CalculateStats()
{
	life_percentage = static_cast<float>(getCurrentHealth()) / static_cast<float>(getModifiedMaxHealth());
	mana_percentage = static_cast<float>(getCurrentMana()) / static_cast<float>(getModifiedMaxMana());
}

bool CCharacter::CheckMouseOver(int _x_pos, int _y_pos)
{
	int myWidth = getWidth();
	int myHeight = getHeight();
	if (((x_pos < _x_pos) && ((x_pos+myWidth) > _x_pos))
	        && (( y_pos < _y_pos) && ((y_pos+myHeight) > _y_pos))) {
		return true;
	} else {
		return false;
	}
}

bool CCharacter::isPlayer() const
{
	return false;
}

void CCharacter::inscribeSpellInSpellbook( CSpellActionBase *spell )
{
    assert( spell != NULL );
    if ( spell->getRequiredClass() == getClass() || spell->getRequiredClass() == CharacterClass::ANYCLASS ) {
        for ( size_t curSpell = 0; curSpell < spellbook.size(); curSpell++ ) {
            if ( spellbook[ curSpell ]->getName() == spell->getName() ) {
                if ( spellbook[ curSpell ]->getRank() < spell->getRank() ) {
                    spellbook[ curSpell ] = spell;
                    if ( isPlayer() == true ) {
                        // this will seed a new ticket for the itemtooltip, causing it to reload. We might need this because of the tooltip message displaying already known spells and ranks.
                        dynamic_cast<Player*>(this)->setTicketForItemTooltip();
                    }
                }
                return;
            }
        }
        spellbook.push_back( spell );

        if ( isPlayer() == true ) {
            // this will seed a new ticket for the itemtooltip, causing it to reload. We might need this because of the tooltip message displaying already known spells and ranks.
            dynamic_cast<Player*>(this)->setTicketForItemTooltip();
        }
    }
}

std::vector<CSpellActionBase*> CCharacter::getSpellbook() const
{
    return spellbook;
}

void CCharacter::setSpellbook( std::vector<CSpellActionBase*> spellbook )
{
    this->spellbook = spellbook;
}

void CCharacter::addActiveSpell( CSpellActionBase *spell )
{
    assert( spell != NULL );
    // here we check to see if the current spell cast is already on the character. if it is, then we refresh it.
    for ( size_t curSpell = 0; curSpell < activeSpells.size(); curSpell++ )
    {
        if ( activeSpells[curSpell].first->getID() == spell->getID() )
        {
            // we replace the old spell with a new, in case a more powerful spell is cast (a higher rank)
            activeSpells[curSpell].first = spell;
            activeSpells[curSpell].second = SDL_GetTicks();
            return;
        }
    }

    // add new spell on character.
    activeSpells.push_back( std::pair<CSpellActionBase*, uint32_t>( spell, SDL_GetTicks() ) );
}

void CCharacter::cleanupActiveSpells()
{
    size_t curSpell = 0;
    while ( curSpell < activeSpells.size() ) {
        if ( activeSpells[curSpell].first->isEffectComplete() == true ) {
            delete activeSpells[curSpell].first;
            activeSpells.erase( activeSpells.begin() + curSpell );
        } else {
            curSpell ++;
        }
    }
}

void CCharacter::clearActiveSpells()
{
    activeSpells.clear();
}

std::vector<std::pair<CSpellActionBase*, uint32_t> > CCharacter::getActiveSpells() const
{
    return activeSpells;
}


void CCharacter::addCooldownSpell( CSpellActionBase *spell )
{
    assert( spell != NULL );
    if ( spell->getCooldown() > 0 )
    {
        cooldownSpells.push_back( std::pair<CSpellActionBase*,uint32_t>( spell, SDL_GetTicks() ) );
    }
}

void CCharacter::cleanupCooldownSpells()
{
    size_t curSpell = 0;
    while ( curSpell < cooldownSpells.size() ) {
        uint32_t thisDuration = SDL_GetTicks();
        if ( thisDuration - cooldownSpells[curSpell].second > cooldownSpells[curSpell].first->getCooldown() * 1000u ) {
            delete cooldownSpells[curSpell].first;
            cooldownSpells.erase( cooldownSpells.begin() + curSpell );
            if ( isPlayer() == true ) {
                // this will seed a new ticket for the itemtooltip and spelltooltips, causing them to reload.
                dynamic_cast<Player*>(this)->setTicketForItemTooltip();
                dynamic_cast<Player*>(this)->setTicketForSpellTooltip();
            }
        } else {
            curSpell ++;
        }
    }
}

void CCharacter::clearCooldownSpells()
{
    cooldownSpells.clear();
}

std::vector<std::pair<CSpellActionBase*, uint32_t> > CCharacter::getCooldownSpells() const
{
    return cooldownSpells;
}

uint32_t CCharacter::getTicksOnCooldownSpell( std::string spellName ) const
{
    for ( size_t curSpell = 0; curSpell < cooldownSpells.size(); curSpell++ )
    {
        if ( cooldownSpells[curSpell].first->getName() == spellName )
        {
            return cooldownSpells[curSpell].second;
        }
    }
    return false;
}

bool CCharacter::isSpellOnCooldown( std::string spellName ) const
{
    for (size_t curSpell = 0; curSpell < cooldownSpells.size(); curSpell++)
    {
	    if ( cooldownSpells[curSpell].first->getName() == spellName )
        {
            return true;
        }
    }
    return false;
}


void CCharacter::giveCoins( uint32_t amountOfCoins )
{
    // should check for maxValue here.
    coins += amountOfCoins;
}

void CCharacter::reduceCoins( uint32_t amountOfCoins )
{
    if ( coins >= amountOfCoins )
    {
        coins -= amountOfCoins;
    } else {
        coins = 0;
    }
}

uint32_t CCharacter::getCoins() const
{
    return coins;
}

void CCharacter::setCoinDrop( uint32_t minCoinDrop, uint32_t maxCoinDrop, double dropChance )
{
	this->minCoinDrop = minCoinDrop;
	this->maxCoinDrop = maxCoinDrop;
	this->coinDropChance = dropChance;
}

void CCharacter::startShopping()
{
    isCurrentlyShopping = true;
}

void CCharacter::stopShopping()
{
    isCurrentlyShopping = false;
}

bool CCharacter::isShopping() const
{
    return isCurrentlyShopping;
}
