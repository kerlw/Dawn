if ( arinoxMagesGuild == nil ) then
	arinoxMagesGuild = {};
--	dofile( "data/quests_lostmushroom.lua" );
	arinoxMagesGuild.leavePoint = DawnInterface.addInteractionPoint();
	arinoxMagesGuild.leavePoint:setPosition( -160, -94, 80, 80 );
	arinoxMagesGuild.leavePoint:setBackgroundTexture( "data/transparent2x2pixel.tga" );
	arinoxMagesGuild.leavePoint:setInteractionType( InteractionType.Zone );
	arinoxMagesGuild.leavePoint:setInteractionCode( 'DawnInterface.enterZone( "data/zone1", 430, 2020 );\nDawnInterface.setSavingAllowed( true );' );

	quest_lostmushroom.baba = DawnInterface.addMobSpawnPoint( "Witch", -344, 180, 1, 0 );
  quest_lostmushroom.baba:setAttitude( Attitude.FRIENDLY );
  quest_lostmushroom.baba:setName( "Baba" );
  quest_lostmushroom.babaInteraction = DawnInterface.addCharacterInteractionPoint( quest_lostmushroom.baba );
  quest_lostmushroom.babaInteraction:setInteractionType( InteractionType.Quest );
  quest_lostmushroom.babaInteraction:setInteractionCode( "quest_lostmushroom.speakWithBaba()" );
	
	--arinoxMagesGuild.traderShop = DawnInterface.addShop();
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["leynorscap"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["smallhealingpotion"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["bladeofstrength"] );
	--arinoxMagesGuild.traderShop:addItem( itemDatabase["bookofleatherskinrank2"] );
end

function arinoxMagesGuild.onEnterMap(x,y)
  local textWindow = DawnInterface.createTextWindow();
  textWindow:center();
  textWindow:setText( "Mages' Guild" );
  textWindow:setAutocloseTime( 1000 );
end
