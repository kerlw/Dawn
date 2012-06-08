if ( arinoxHut == nil ) then
  arinoxHut = {};
  --dofile("data/arinoxGeneralShop.spawnpoints.lua");
  arinoxHut.leavePoint = DawnInterface.addInteractionPoint();
  arinoxHut.leavePoint:setPosition( -160, -94, 80, 80 );
  arinoxHut.leavePoint:setBackgroundTexture( "data/transparent2x2pixel.tga" );
  arinoxHut.leavePoint:setInteractionType( InteractionType.Zone );
  arinoxHut.leavePoint:setInteractionCode( 'DawnInterface.enterZone( "data/zone1", 1760, 930 );\nDawnInterface.setSavingAllowed( true );' );
end

function arinoxHut.onEnterMap(x,y)

end
