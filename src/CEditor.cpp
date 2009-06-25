/**                    Dawn - 2D roleplaying game
    Copyright (C) 2009  Tobias Eliasson (arnestig@users.sourceforge.net)

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

#include "CEditor.h"
#include "GLFT_Font.h"

#include "CDrawingHelpers.h"

void CEditor::inc_tilepos() {
    switch (current_object) {
        case 0: // tiles
            if (current_tilepos < zone1.ZoneTiles.NumberOfTextures) {
                current_tilepos++;
                tilepos_offset--;
            }
            break;
        case 1: // environment
            if (current_tilepos < zone1.ZoneEnvironment.NumberOfTextures) {
                current_tilepos++;
                tilepos_offset--;
            }
            break;
        case 2: // shadows
            if (current_tilepos < zone1.ZoneShadow.NumberOfTextures) {
                current_tilepos++;
                tilepos_offset--;
            }
            break;
        case 3: // collisionboxes
            break;
    }
}

void CEditor::dec_tilepos() {
    if (current_tilepos > 1) {
        current_tilepos--;
        tilepos_offset++;
    }
}

int CEditor::SaveZone() {
    FILE *fp;
    // open the tilemap-file, so we can write our new mapfile ;D ///////////////////////////
    if ((fp=fopen("data/zone1.tilemap", "w")) == NULL) {
        printf("ERROR opening file zone1.tilemap\n\n");
        return -1;
    }
    fprintf(fp,"#x-pos y-pos tile-id");

    for (unsigned int x=0; x<zone1.TileMap.size();x++) {
        fprintf(fp,"\n%d %d %d",zone1.TileMap[x].x_pos,zone1.TileMap[x].y_pos,zone1.TileMap[x].id);
    }

    fclose(fp);
    /////////////////////////////////////////////////////////////////////////////////////

    // open the environmentmap-file, so we can write our trees and stuff...
    if ((fp=fopen("data/zone1.environmentmap", "w")) == NULL) {
        printf("ERROR opening file zone1.environmentmap\n\n");
        return -1;
    }
    fprintf(fp,"#x-pos y-pos tile-id transparency red green blue x_scale y_scale");


    for (unsigned int x=0;x<zone1.EnvironmentMap.size();x++) {
        fprintf(fp,"\n%d %d %d %.2f %.2f %.2f %.2f %.2f %.2f",zone1.EnvironmentMap[x].x_pos,zone1.EnvironmentMap[x].y_pos, zone1.EnvironmentMap[x].id, zone1.EnvironmentMap[x].transparency,zone1.EnvironmentMap[x].red,zone1.EnvironmentMap[x].green,zone1.EnvironmentMap[x].blue, zone1.EnvironmentMap[x].x_scale, zone1.EnvironmentMap[x].y_scale);
    }
    fclose(fp);
    ////////////////////////////////////////////////////////////////////////


    // open the shadowmap-file, so we can save our shadow...
    if ((fp=fopen("data/zone1.shadowmap", "w")) == NULL) {
        printf("ERROR opening file zone1.shadowmap\n\n");
        return -1;
    }
    fprintf(fp,"#x-pos y-pos tile-id transparency red green blue x_scale y_scale");

    for (unsigned int x=0;x<zone1.ShadowMap.size();x++) {
        fprintf(fp,"\n%d %d %d %.2f %.2f %.2f %.2f %.2f %.2f",zone1.ShadowMap[x].x_pos,zone1.ShadowMap[x].y_pos, zone1.ShadowMap[x].id, zone1.ShadowMap[x].transparency, zone1.ShadowMap[x].red, zone1.ShadowMap[x].green, zone1.ShadowMap[x].blue, zone1.ShadowMap[x].x_scale, zone1.ShadowMap[x].y_scale);
    }
    fclose(fp);
    /////////////////////////////////////////////////////////

    // open the collisionmap-file, so we can save our shadow...
    if ((fp=fopen("data/zone1.collisionmap", "w")) == NULL) {
        printf("ERROR opening file zone1.collisionmap\n\n");
        return -1;
    }
    fprintf(fp,"#x y h w");

    for (unsigned int x=0;x<zone1.CollisionMap.size();x++) {
        fprintf(fp,"\n%d %d %d %d",zone1.CollisionMap[x].CR.x,zone1.CollisionMap[x].CR.y, zone1.CollisionMap[x].CR.h, zone1.CollisionMap[x].CR.w);
    }
    fclose(fp);
    /////////////////////////////////////////////////////////
    return 0;
};

void CEditor::HandleKeys() {
    Uint8 *keys;
    keys = SDL_GetKeyState(NULL);
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)  { done = 1; }

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) { done = 1; }
            if (event.key.keysym.sym == SDLK_SPACE) { }
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            switch (event.button.button) {
                case 1: // mouse button 1, see if we can select an object being pointed at.
                switch (current_object) {
                    case 1: // environment
                    objectedit_selected = zone1.LocateEnvironment(world_x+mouseX,world_y+mouseY);
                    break;
                    case 2: // shadows
                    objectedit_selected = zone1.LocateShadow(world_x+mouseX,world_y+mouseY);
                    break;
                    case 3: // collisionboxes
                    objectedit_selected = zone1.LocateCollisionbox(world_x+mouseX,world_y+mouseY);
                    break;
                }
                break;

                case 4: // scroll up, increase our tileposition
                inc_tilepos();
                break;

                case 5: // scroll down, decrease our tileposition
                dec_tilepos();
                break;
            }
        }

        if (event.type == SDL_MOUSEMOTION) {
            mouseX = event.motion.x;
            mouseY = RES_Y - event.motion.y - 1;
        }
    }

    // the arrow-keys. if an object is selected, we move it around. if Left shift is pushed, we scale the object..
    // else we move around in our zone.

    if (objectedit_selected >= 0) {
        switch (current_object) {
            case 1: // environment
                if (keys[SDLK_LSHIFT]) {
                    if (keys[SDLK_DOWN]) { zone1.EnvironmentMap[objectedit_selected].y_scale -= 0.01f; }
                    if (keys[SDLK_UP]) { zone1.EnvironmentMap[objectedit_selected].y_scale += 0.01f; }
                    if (keys[SDLK_LEFT]) { zone1.EnvironmentMap[objectedit_selected].x_scale -= 0.01f; }
                    if (keys[SDLK_RIGHT]) { zone1.EnvironmentMap[objectedit_selected].x_scale += 0.01f; }
                } else {
                    if (keys[SDLK_DOWN]) { zone1.EnvironmentMap[objectedit_selected].y_pos--; }
                    if (keys[SDLK_UP]) { zone1.EnvironmentMap[objectedit_selected].y_pos++; }
                    if (keys[SDLK_LEFT]) { zone1.EnvironmentMap[objectedit_selected].x_pos--; }
                    if (keys[SDLK_RIGHT]) { zone1.EnvironmentMap[objectedit_selected].x_pos++; }
                }
                break;
            case 2: // shadows
                if (keys[SDLK_LSHIFT]) {
                    if (keys[SDLK_DOWN]) { zone1.ShadowMap[objectedit_selected].y_scale -= 0.01f; }
                    if (keys[SDLK_UP]) { zone1.ShadowMap[objectedit_selected].y_scale += 0.01f; }
                    if (keys[SDLK_LEFT]) { zone1.ShadowMap[objectedit_selected].x_scale -= 0.01f; }
                    if (keys[SDLK_RIGHT]) { zone1.ShadowMap[objectedit_selected].x_scale += 0.01f; }
                } else {
                    if (keys[SDLK_DOWN]) { zone1.ShadowMap[objectedit_selected].y_pos--; }
                    if (keys[SDLK_UP]) { zone1.ShadowMap[objectedit_selected].y_pos++; }
                    if (keys[SDLK_LEFT]) { zone1.ShadowMap[objectedit_selected].x_pos--; }
                    if (keys[SDLK_RIGHT]) { zone1.ShadowMap[objectedit_selected].x_pos++; }
                }
                break;
            case 3: // collisionboxes
                if (keys[SDLK_LSHIFT]) {
                    if (keys[SDLK_DOWN]) { zone1.CollisionMap[objectedit_selected].CR.h -= 1; }
                    if (keys[SDLK_UP]) { zone1.CollisionMap[objectedit_selected].CR.h += 1; }
                    if (keys[SDLK_LEFT]) { zone1.CollisionMap[objectedit_selected].CR.w -= 1; }
                    if (keys[SDLK_RIGHT]) { zone1.CollisionMap[objectedit_selected].CR.w += 1; }
                } else {
                    if (keys[SDLK_DOWN]) { zone1.CollisionMap[objectedit_selected].CR.y--; }
                    if (keys[SDLK_UP]) { zone1.CollisionMap[objectedit_selected].CR.y++; }
                    if (keys[SDLK_LEFT]) { zone1.CollisionMap[objectedit_selected].CR.x--; }
                    if (keys[SDLK_RIGHT]) { zone1.CollisionMap[objectedit_selected].CR.x++; }
                }
                break;
        }
    } else {
        if (keys[SDLK_DOWN]) { world_y -= 3; }
        if (keys[SDLK_UP]) { world_y += 3; }
        if (keys[SDLK_LEFT]) { world_x -= 3; }
        if (keys[SDLK_RIGHT]) { world_x += 3; }
    }

    if (keys[SDLK_DELETE] && !KP_delete_environment) {
        KP_delete_environment = true;
        switch (current_object) {
            case 0: // tiles
                zone1.DeleteTile(zone1.LocateTile(world_x+mouseX,world_y+mouseY));
                break;
            case 1: // environment
                zone1.DeleteEnvironment(world_x+mouseX,world_y+mouseY);
                break;
            case 2: // shadows
                zone1.DeleteShadow(world_x+mouseX,world_y+mouseY);
                break;
            case 3: // collisionboxes
                zone1.DeleteCollisionbox(world_x+mouseX,world_y+mouseY);
                break;
        }
    }

    if (!keys[SDLK_DELETE]) {
        KP_delete_environment = false;
    }

    if (keys[SDLK_KP_ENTER] && !KP_add_environment) {
        objectedit_selected = -1;
        KP_add_environment = true;
        switch (current_object) {
            case 0: // tiles
                zone1.ChangeTile(zone1.LocateTile(world_x+mouseX,world_y+mouseY),current_tilepos);
                break;
            case 1: // environment
                zone1.AddEnvironment(world_x+mouseX,world_y+mouseY,current_tilepos);
                break;
            case 2: // shadows
                zone1.AddShadow(world_x+mouseX,world_y+mouseY,current_tilepos);
                break;
            case 3: // collisionboxes
                zone1.AddCollisionbox(world_x+mouseX,world_y+mouseY);
                break;
        }
    }

    if (!keys[SDLK_KP_ENTER]) {
        KP_add_environment = false;
    }

    if (keys[SDLK_l] && !KP_toggle_editor) {
        current_tilepos = 1;
        tilepos_offset = 0;
        objectedit_selected = -1;
        enabled = false;
        KP_toggle_editor = true;
    }

    if (!keys[SDLK_l]) {
        KP_toggle_editor = false;
    }

    if (keys['.']) {  // increase the amount of transparency of the object.
        switch (current_object) {
            case 1: // environment
                if (zone1.EnvironmentMap[objectedit_selected].transparency > 0.01f) {
                    zone1.EnvironmentMap[objectedit_selected].transparency -= 0.01f;
                }
                break;
            case 2: // shadows
                if (zone1.ShadowMap[objectedit_selected].transparency > 0.01f) {
                    zone1.ShadowMap[objectedit_selected].transparency -= 0.01f;
                }
                break;
        }
    }

    if (keys[',']) { // decrease the amount of transparency of the object.
        switch (current_object) {
            case 1: // environment
                if (zone1.EnvironmentMap[objectedit_selected].transparency < 0.99f) {
                    zone1.EnvironmentMap[objectedit_selected].transparency += 0.01f;
                }
                break;
            case 2: // shadows
                if (zone1.ShadowMap[objectedit_selected].transparency < 0.99f) {
                    zone1.ShadowMap[objectedit_selected].transparency += 0.01f;
                }
                break;
        }
    }

    // change the objects red color
    if (keys['1']){
        switch (current_object) {
            case 1:
                if (keys[SDLK_LSHIFT]) {
                    if (zone1.EnvironmentMap[objectedit_selected].red > 0.01f) {
                        zone1.EnvironmentMap[objectedit_selected].red -= 0.01f;
                    }
                } else {
                    if (zone1.EnvironmentMap[objectedit_selected].red < 1.0f) {
                        zone1.EnvironmentMap[objectedit_selected].red += 0.01f;
                    }
                }
                break;
            case 2:
                if (keys[SDLK_LSHIFT]) {
                    if (zone1.ShadowMap[objectedit_selected].red > 0.01f) {
                        zone1.ShadowMap[objectedit_selected].red -= 0.01f;
                    }
                } else {
                    if (zone1.ShadowMap[objectedit_selected].red < 1.0f) {
                        zone1.ShadowMap[objectedit_selected].red += 0.01f;
                    }
                }
                break;
        }
    }

    // change the objects green color
    if (keys['2']){
        switch (current_object) {
            case 1:
                if (keys[SDLK_LSHIFT]) {
                    if (zone1.EnvironmentMap[objectedit_selected].green > 0.01f) {
                        zone1.EnvironmentMap[objectedit_selected].green -= 0.01f;
                    }
                } else {
                    if (zone1.EnvironmentMap[objectedit_selected].green < 1.0f) {
                        zone1.EnvironmentMap[objectedit_selected].green += 0.01f;
                    }
                }
                break;
            case 2:
                if (keys[SDLK_LSHIFT]) {
                    if (zone1.ShadowMap[objectedit_selected].green > 0.01f) {
                        zone1.ShadowMap[objectedit_selected].green -= 0.01f;
                    }
                } else {
                    if (zone1.ShadowMap[objectedit_selected].green < 1.0f) {
                        zone1.ShadowMap[objectedit_selected].green += 0.01f;
                    }
                }
                break;
        }
    }

    // change the objects blue color
    if (keys['3']){
        switch (current_object) {
            case 1:
                if (keys[SDLK_LSHIFT]) {
                    if (zone1.EnvironmentMap[objectedit_selected].blue > 0.01f) {
                        zone1.EnvironmentMap[objectedit_selected].blue -= 0.01f;
                    }
                } else {
                    if (zone1.EnvironmentMap[objectedit_selected].blue < 1.0f) {
                        zone1.EnvironmentMap[objectedit_selected].blue += 0.01f;
                    }
                }
                break;
            case 2:
                if (keys[SDLK_LSHIFT]) {
                    if (zone1.ShadowMap[objectedit_selected].blue > 0.01f) {
                        zone1.ShadowMap[objectedit_selected].blue -= 0.01f;
                    }
                } else {
                    if (zone1.ShadowMap[objectedit_selected].blue < 1.0f) {
                        zone1.ShadowMap[objectedit_selected].blue += 0.01f;
                    }
                }
                break;
        }
    }

    if (keys[SDLK_F1] && !KP_toggle_tileset) {
        current_tilepos = 1;
        tilepos_offset = 0;
        objectedit_selected = -1;

        KP_toggle_tileset = true;
        if (current_object < 3) {
            current_object++;
        } else {
            current_object = 0;
        }
    }

    if (!keys[SDLK_F1]) {
        KP_toggle_tileset = false;
    }

    if (keys[SDLK_s] && !KP_save_zone) {
        KP_save_zone = true;
        SaveZone();
        message.AddText(world_x + (RES_X/2), world_y + (RES_Y/2), 1.0f, 0.625f, 0.71f, 1.0f, 15, 3.0f, "Zone saved ...");
    }

    if (!keys[SDLK_s]) {
        KP_save_zone = false;
    }

};

void CEditor::DrawEditor() {
    if (current_object == 3) {
        // we have selected to work with collisionboxes, draw them.
        for (unsigned int x = 0; x < zone1.CollisionMap.size(); x++) {
            if (objectedit_selected == (signed int)x) { // if we have a selected collisionbox, draw it a little brighter than the others.
                glColor4f(1.0f, 1.0f, 1.0f,1.0f);
            } else {
                glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
            }

            DrawingHelpers::mapTextureToRect( interfacetexture.texture[1].texture,
                                              zone1.CollisionMap[x].CR.x, zone1.CollisionMap[x].CR.w,
                                              zone1.CollisionMap[x].CR.y, zone1.CollisionMap[x].CR.h );
            glColor4f(1.0f,1.0f,1.0f,1.0f);
        }
    }

    if (objectedit_selected >= 0) { // we have selected an object to edit it's properties, show the edit-screen.
        switch (current_object) {
            case 1:
            DrawEditFrame(&zone1.EnvironmentMap[objectedit_selected], &zone1.ZoneEnvironment, objectedit_selected);
            break;
            case 2:
            DrawEditFrame(&zone1.ShadowMap[objectedit_selected], &zone1.ZoneShadow, objectedit_selected);
            break;
            case 3:
            break;
        }
    }

    glEnable(GL_BLEND);
    // quad on the top, baseframe for the object-selection.
    DrawingHelpers::mapTextureToRect( interfacetexture.texture[0].texture,
                                      world_x, RES_X,
                                      world_y+RES_Y-100, 100 );

    // quad on bottom, baseframe for our helptext.
    DrawingHelpers::mapTextureToRect( interfacetexture.texture[0].texture,
                                      world_x, RES_X,
                                      world_y, 100 );

    GLFT_Font fnt("data/verdana.ttf", 9);
    int fontHeight = fnt.getHeight();

    // display our general help text for the editor.
    glColor4f(1.0f,1.0f,0.13f,1.0f); // set yellow as font color
    fnt.drawText(world_x+10, world_y+90 - fontHeight, "[ Scoll Up/Down ]  Select previous/next object");
    fnt.drawText(world_x+10, world_y+80 - fontHeight, "[ F1 ]  Next set of objects");
    fnt.drawText(world_x+10, world_y+70 - fontHeight, "[ DEL ]  Delete object at mouse position");
    fnt.drawText(world_x+10, world_y+60 - fontHeight, "[ ENTER ]  Place object at mouse position");
    fnt.drawText(world_x+10, world_y+50 - fontHeight, "[ S ]  Saves the changes into zone1-files");
    fnt.drawText(world_x+10, world_y+40 - fontHeight, "[ O ]  Load a different zone (not yet implemented)");
    fnt.drawText(world_x+10, world_y+30 - fontHeight, "[ L ]  Exit the editor");

    // if we have a selected object, display specific help text for it
    if (objectedit_selected >= 0) {
        glColor4f(0.5f,1.0f,0.5f,1.0f);
        fnt.drawText(world_x+500, world_y+90 - fontHeight, "[ UP, DOWN, LEFT, RIGHT ]  Move the object");
        fnt.drawText(world_x+500, world_y+80 - fontHeight, "[ Left Shift + UP, DOWN, LEFT, RIGHT ]  Change scale of object");
        fnt.drawText(world_x+500, world_y+70 - fontHeight, "[ . ]  Increase transparency");
        fnt.drawText(world_x+500, world_y+60 - fontHeight, "[ , ]  Decrease transparency");
        fnt.drawText(world_x+500, world_y+50 - fontHeight, "[ 1/2/3 ]  Increase color RED/GREEN/BLUE");
        fnt.drawText(world_x+500, world_y+40 - fontHeight, "[ Left Shift + 1/2/3 ]  Decrease color RED/GREEN/BLUE)");
    }

    glColor4f(1.0f,1.0f,1.0f,1.0f); // and back to white.

    DrawingHelpers::mapTextureToRect( interfacetexture.texture[1].texture,
                                      world_x+(RES_X/2)-5, 50,
                                      world_y+RES_Y-65, 50 );

    glBegin(GL_LINES);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(mouseX-20+world_x, mouseY+world_y, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(mouseX+20+world_x, mouseY+world_y, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(mouseX+world_x, mouseY+20+world_y, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(mouseX+world_x, mouseY-20+world_y, 0.0f);
    glEnd();

    switch (current_object) {
        case 0:
            // draw all tileset tiles in edit frame
            for (tilepos=1;tilepos<=zone1.ZoneTiles.NumberOfTextures;tilepos++) {

                DrawingHelpers::mapTextureToRect( zone1.ZoneTiles.texture[tilepos].texture,
                                                  world_x+(RES_X/2)-50+(tilepos*50)+(tilepos_offset*50), 40,
                                                  world_y+RES_Y-60, 40 );
            }
            break;

        case 1:
            // draw all environment objects in edit frame
            for (tilepos=1;tilepos<=zone1.ZoneEnvironment.NumberOfTextures; tilepos++) {

                DrawingHelpers::mapTextureToRect( zone1.ZoneEnvironment.texture[tilepos].texture,
                                                  world_x+(RES_X/2)-50+(tilepos*50)+(tilepos_offset*50), 40,
                                                  world_y+RES_Y-60, 40 );
            }
            break;

        case 2:
            // draw all available shadows in edit frame 
            for (tilepos=1;tilepos<=zone1.ZoneShadow.NumberOfTextures; tilepos++) {

                DrawingHelpers::mapTextureToRect( zone1.ZoneShadow.texture[tilepos].texture,
                                                  world_x+(RES_X/2)-50+(tilepos*50)+(tilepos_offset*50), 40,
                                                  world_y+RES_Y-60, 40 );
            }
            break;

        case 3:
            break;

    }
    glDisable(GL_BLEND);
};

void CEditor::LoadTextures() {
    interfacetexture.texture.reserve(4);
    interfacetexture.LoadIMG("data/background_editor.tga",0);
    interfacetexture.LoadIMG("data/current_tile_backdrop.tga",1);
    interfacetexture.LoadIMG("data/tile.tga",2);
    interfacetexture.LoadIMG("data/edit_backdrop.tga",3);
}

void CEditor::DrawEditFrame(sEnvironmentMap *editobject, CTexture *texture, int object_id) {
    glEnable(GL_BLEND);
    // draws a white quad as our editframe
    DrawingHelpers::mapTextureToRect( interfacetexture.texture[3].texture,
                                      world_x+50, 350,
                                      world_y+(RES_Y/2)-200, 200 );


    // set the color, transparency, scale and then draws the object we are editing
    glPushMatrix();
    glScalef(editobject->x_scale,editobject->y_scale,1.0f);
    glColor4f(editobject->red, editobject->green, editobject->blue, editobject->transparency);
    
    DrawingHelpers::mapTextureToRect( texture->texture[editobject->id].texture,
                                      world_x+55, texture->texture[editobject->id].width,
                                      world_y+(RES_Y/2)-texture->texture[editobject->id].height-5, texture->texture[editobject->id].height );
    
    glPopMatrix();

    glColor4f(0.0f,0.0f,0.0f,1.0f);
    GLFT_Font fnt("data/verdana.ttf", 10);
    int fontHeight = fnt.getHeight();

    fnt.drawText(world_x+242, world_y+(RES_Y/2)-10 - fontHeight, "Transparency: %.2f",editobject->transparency);
    fnt.drawText(world_x+319, world_y+(RES_Y/2)-22 - fontHeight, "Red: %.2f",editobject->red);
    fnt.drawText(world_x+300, world_y+(RES_Y/2)-34 - fontHeight, "Green: %.2f",editobject->green);
    fnt.drawText(world_x+312, world_y+(RES_Y/2)-46 - fontHeight, "Blue: %.2f",editobject->blue);
    fnt.drawText(world_x+287, world_y+(RES_Y/2)-58 - fontHeight, "Scale X: %.2f",editobject->x_scale);
    fnt.drawText(world_x+287, world_y+(RES_Y/2)-70 - fontHeight, "Scale Y: %.2f",editobject->y_scale);

    glDisable(GL_BLEND);
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    glScalef(1.0f,1.0f,1.0f);
}
