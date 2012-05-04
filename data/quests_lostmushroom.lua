-- init quest namespace so different quests / zones don't conflict in names
if( quest_lostmushroom == nil )
then
  quest_lostmushroom = {}
end

function quest_lostmushroom.init()
  -- this quest is initialized from zone arinoxMagesGuild
  
end

function quest_lostmushroom.speakWithBaba()
  if( not quest_lostmushroom.completed ) then
    if( quest_lostmushroom.added == nil ) then
      quest_lostmushroom.added = true;
      quest_lostmushroom.completed = false;
      quest_lostmushroom.gotReward = false;
      quest_lostmushroom.gotMushroom = false;
      quest_lostmushroom.mushroomSpawned = false;
      quest_lostmushroom.quest = DawnInterface.addQuest( "Lost Mushroom", "The witch Baba has lost her mushroom. Locate it in the Syphir Woods and bring it back to her." );
      quest_lostmushroom.quest:setExperienceReward( 400 );
      quest_lostmushroom.quest:setCoinReward( 1000 );
      quest_lostmushroom.quest:addRequiredItemForCompletion( itemDatabase["mysticmushroom"], 1 );
      quest_lostmushroom.showBabaText( 1 );
    elseif ( quest_lostmushroom.added == true and quest_lostmushroom.completed == false and quest_lostmushroom.gotMushroom == false ) then
      quest_lostmushroom.showBabaText( 2 );
    elseif ( quest_lostmushroom.gotMushroom == true ) then
      quest_lostmushroom.showBabaText( 3 );
      quest_lostmushroom.completed = true;
      quest_lostmushroom.quest:finishQuest();
      quest_lostmushroom.quest = nil;
    end
  else
    quest_lostmushroom.showBabaText( 4 );
  end  
end

function quest_lostmushroom.showBabaText( part )
  if( part == 1 ) then
    local textWindow = DawnInterface.createTextWindow();
    textWindow:setPosition( PositionType.CENTER, 512, 382 );
    textWindow:setText( "Please bring me my mushroom. I lost it in the Sylphir Woods." );
    textWindow:setAutocloseTime( 0 );
  end
  if( part == 2 ) then
    local textWindow = DawnInterface.createTextWindow();
    textWindow:setPosition( PositionType.CENTER, 512, 382 );
    textWindow:setText( "Did you find it already?" );
    textWindow:setAutocloseTime( 0 );
  end
  if( part == 3 ) then
    local textWindow = DawnInterface.createTextWindow();
    textWindow:setPosition( PositionType.CENTER, 512, 382 );
    textWindow:setText( "Oh, you really did find it! Thank you my friend." );
    textWindow:setAutocloseTime( 0 );
  end
  if( part == 4 ) then
    local textWindow = DawnInterface.createTextWindow();
    textWindow:setPosition( PositionType.CENTER, 512, 382 );
    textWindow:setText( "Thank you for finding my mushroom!" );
    textWindow:setAutocloseTime( 0 );
  end
end

function quest_lostmushroom.onMushroomPickup()
  DawnInterface.giveItemToPlayer( itemDatabase["mysticmushroom"] );
  quest_lostmushroom.gotMushroom = true;
  quest_lostmushroom.quest:setDescription( "You've found the mushroom. Return to Baba." );
  DawnInterface.removeInteractionPoint( quest_lostmushroom.mushroom );
  quest_lostmushroom.mushroom = nil;
end

-- init quest if this has not been done yet
if( quest_lostmushroom.inited == nil ) then
  quest_lostmushroom.inited = true;
  quest_lostmushroom.init();
end
