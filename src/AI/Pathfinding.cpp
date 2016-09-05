//
//  Pathfinding.cpp
//  A4Engine
//
//  Created by Santiago Ontanon on 4/12/15.
//  Copyright (c) 2015 Santiago Ontanon. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#else
#include "unistd.h"
#include "sys/stat.h"
#include "sys/types.h"
#endif

#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include "math.h"

#include "SDL.h"
#ifdef __EMSCRIPTEN__
#include "SDL/SDL_opengl.h"
#include <glm.hpp>
#include <ext.hpp>
#else
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#ifdef __APPLE__
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#else
#include "GL/glew.h"
#include "GL/gl.h"
#include "GL/glu.h"
#endif
#endif
#include "SDLauxiliar.h"
#include "GLauxiliar.h"

#include <vector>
#include "sound.h"
#include "keyboardstate.h"
#include "Symbol.h"
#include "GLTile.h"
#include "GLTManager.h"
#include "BitmapFont.h"
#include "Binterface.h"
#include "XMLparser.h"
#include "Animation.h"

#include "A4Script.h"
#include "A4EventRule.h"
#include "A4Map.h"
#include "A4Object.h"
#include "A4Game.h"

#include "A4Character.h"
#include "A4AICharacter.h"
#include "A4Vehicle.h"

#include "A4Behavior.h"

#include "A4AI.h"


A4CharacterCommand *A4AI::navigationCycle(A4Game *game)
{
    A4WalkingObject *subject = m_character;
    if (m_character->isInVehicle()) subject = m_character->getVehicle();
    A4CharacterCommand *command = 0;
    int m_highest_priority = 0;

    if (m_pathfinding_n_targets==0) return 0;

    // FIRST: check if near pixel-level pathfinding is enough:
    for(int i = 0;i<m_pathfinding_n_targets;i++)
        if (m_pathfinding_targets[i].m_priority>m_highest_priority) m_highest_priority = m_pathfinding_targets[i].m_priority;
    command = pixelLevelPathFinding(subject);
    // if the command is directed to the highest priority goal alrady, then there is no need to path find!
    if (command!=0) {        
        if (command->m_priority == m_highest_priority) {
            m_pathfinding_result_x = -1;
            m_pathfinding_result_y = -1;
            m_pathfinding_result_priority = 0;
            return command;
        } else {
            delete command;
        }
    }

    // SECOND: if pixel-level pathfinding was not enough, then consider tile-level pathfinding:
    if (m_nextPathFindingTimer>0) m_nextPathFindingTimer--;
    int timeout = subject->getWalkSpeed()*2;
    bool pathfind = false;
    if (m_pathfinding_result_x==-1) {
        pathfind = true;
    } else if (m_pathfinding_result_x!=-1) {
        if ((m_cycle - m_pathFinding_lastUpdated)>timeout) {
            pathfind = true;
        } else {
            int pixelx = subject->getX();
            int pixely = subject->getY();
            int pixel_target_x = m_pathfinding_result_x * tileWidth;
            int pixel_target_y = m_pathfinding_result_y * tileHeight;
            m_pathfinding_result_offset_x = pixel_target_x - pixelx;
            m_pathfinding_result_offset_y = pixel_target_y - pixely;
            if (m_pathfinding_result_offset_x==0 && m_pathfinding_result_offset_y==0) pathfind = true;
        }
    }

    if (m_nextPathFindingTimer==0) pathfind = true;
    if (pathfind) {
        m_pathfinding_result_x = -1;
        m_pathfinding_result_y = -1;
        m_pathfinding_result_priority = 0;
        if (m_navigationBuffer_lastUpdated < m_cycle) updateNavigationPerceptionBuffer(true);
        pathFinding(subject);
        m_nextPathFindingTimer = subject->getWalkSpeed();
    }

    // THIRD: if we did pathfind, then follow the path:
    if (m_pathfinding_result_x!=-1) {
        int pixelx = subject->getX();
        int pixely = subject->getY();
        int pixel_target_x = m_pathfinding_result_x * tileWidth;
        int pixel_target_y = m_pathfinding_result_y * tileHeight;
        m_pathfinding_result_offset_x = pixel_target_x - pixelx;
        m_pathfinding_result_offset_y = pixel_target_y - pixely;
        int abs_diff_x = (m_pathfinding_result_offset_x<0 ? -m_pathfinding_result_offset_x:m_pathfinding_result_offset_x);
        int abs_diff_y = (m_pathfinding_result_offset_y<0 ? -m_pathfinding_result_offset_y:m_pathfinding_result_offset_y);
        if (abs_diff_x>0 && (abs_diff_x<=abs_diff_y || abs_diff_y==0)) {
            if (m_pathfinding_result_offset_x>0) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_x, A4_DIRECTION_RIGHT, 0, m_pathfinding_result_priority);
            } else {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_x, A4_DIRECTION_LEFT, 0, m_pathfinding_result_priority);
            }
        } else if (abs_diff_y>0) {
            if (m_pathfinding_result_offset_y>0) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_y, A4_DIRECTION_DOWN, 0, m_pathfinding_result_priority);
            } else {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_y, A4_DIRECTION_UP, 0, m_pathfinding_result_priority);
            }
        } else {
            // we are at the goal, stay!
            return new A4CharacterCommand(A4CHARACTER_COMMAND_IDLE, 0, 0, 0, m_pathfinding_result_priority);
        }
    }

    return 0;
}


/*
    - This function jut considers targets that are less than a small number of pixels away in each dimension.
    - Only "goto" targets are considered here, since "flee" targets are better handled by the blobal pathfinder
*/
A4CharacterCommand *A4AI::pixelLevelPathFinding(A4Object *subject) {
    int pixelx0 = subject->getX();
    int pixely0 = subject->getY();
    int pixelx1 = pixelx0 + subject->getPixelWidth();
    int pixely1 = pixely0 + subject->getPixelHeight();
    // pixel-level pathfinder:
    int goto_best = -1;
    int goto_best_priority = 0;
    int goto_best_distance = 0;
    for(int i = 0;i<m_pathfinding_n_targets;i++) {
        if (m_pathfinding_targets[i].m_flee) continue;
        if (goto_best!=-1 && goto_best_priority>m_pathfinding_targets[i].m_priority) continue;
        int dx = 0;
        int dy = 0;
        if (pixelx1 < m_pathfinding_targets[i].m_x0) dx = m_pathfinding_targets[i].m_x0 - pixelx1;
        if (pixelx0 > m_pathfinding_targets[i].m_x1) dx = pixelx0 - m_pathfinding_targets[i].m_x1;
        if (pixely1 < m_pathfinding_targets[i].m_y0) dy = m_pathfinding_targets[i].m_y0 - pixely1;
        if (pixely0 > m_pathfinding_targets[i].m_y1) dy = pixely0 - m_pathfinding_targets[i].m_y1;
        int distance = dx+dy;
        if (dx<tileWidth && dy<tileHeight) {
            if (goto_best==-1 || distance<goto_best_distance) {
//                A4Object *target_object = m_pathfinding_targets[i].m_target;
//                if (target_object!=0 && !m_navigationBuffer_map->contains(target_object)) continue;
                goto_best = i;
                goto_best_priority = m_pathfinding_targets[i].m_priority;
                goto_best_distance = distance;
            }
        }
    }
    if (goto_best != -1) {
        // There is a goto target nearby!
        int slack = 0;
        A4Object *target_object = m_pathfinding_targets[goto_best].m_target;
        int x = (m_pathfinding_targets[goto_best].m_x0 + m_pathfinding_targets[goto_best].m_x1)/2;
        int y = (m_pathfinding_targets[goto_best].m_y0 + m_pathfinding_targets[goto_best].m_y1)/2;
        int dx = (x - (pixelx0+pixelx1)/2);
        int dy = (y - (pixely0+pixely1)/2);
        int abs_diff_x = dx>0 ? dx:-dx;
        int abs_diff_y = dy>0 ? dy:-dy;
        bool checkForBridges = false;
        if (target_object!=0 && subject->getMap() == target_object->getMap()) checkForBridges = true;
        // otherwise, just move first in the smalleer difference direction:
        if (abs_diff_x>0 && (abs_diff_x<=abs_diff_y || abs_diff_y==0)) {
            // prefer to move in the X axis first, and also prefer to move to a direction that there is no wall:
            if (dx>0 && subject->canMove(A4_DIRECTION_RIGHT, slack, checkForBridges)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_x, A4_DIRECTION_RIGHT, 0, goto_best_priority);
            } else if (dx<0 && subject->canMove(A4_DIRECTION_LEFT, slack, checkForBridges)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_x, A4_DIRECTION_LEFT, 0, goto_best_priority);
            } else if (dy>0 && subject->canMove(A4_DIRECTION_DOWN, slack, checkForBridges)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_y, A4_DIRECTION_DOWN, 0, goto_best_priority);
            } else if (dy<0 && subject->canMove(A4_DIRECTION_UP, slack, checkForBridges)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_y, A4_DIRECTION_UP, 0, goto_best_priority);
            }
            if (m_pathfinding_targets[goto_best].m_action == A4CHARACTER_COMMAND_ATTACK) {
                // look if we can attack:
                A4Object *target = m_pathfinding_targets[goto_best].m_target;
                if (target!=0) {
                    if (pixelx1==m_pathfinding_targets[goto_best].m_x0)
                        return new A4CharacterCommand(A4CHARACTER_COMMAND_ATTACK, 0, A4_DIRECTION_RIGHT, target, 0, goto_best_priority);
                    if (pixelx0==m_pathfinding_targets[goto_best].m_x1)
                        return new A4CharacterCommand(A4CHARACTER_COMMAND_ATTACK, 0, A4_DIRECTION_LEFT, target, 0, goto_best_priority);
                    if (pixely1==m_pathfinding_targets[goto_best].m_y0)
                        return new A4CharacterCommand(A4CHARACTER_COMMAND_ATTACK, 0, A4_DIRECTION_DOWN, target, 0, goto_best_priority);
                    if (pixely0==m_pathfinding_targets[goto_best].m_y1)
                        return new A4CharacterCommand(A4CHARACTER_COMMAND_ATTACK, 0, A4_DIRECTION_UP, target, 0, goto_best_priority);
                }
            }
            // However, if there is no alternative, at least try to stand still , since we are so close!
            // This prevents other behaviors from taking over movement, if this is the highest priority target:
            return new A4CharacterCommand(A4CHARACTER_COMMAND_IDLE, 0, 0, 0, goto_best_priority);
/*
            if (dx>0) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_x, A4_DIRECTION_RIGHT, 0, goto_best_priority);
            } else {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_x, A4_DIRECTION_LEFT, 0, goto_best_priority);
            }
*/
        } else if (abs_diff_y>0) {
            if (dy>0 && subject->canMove(A4_DIRECTION_DOWN, slack, checkForBridges)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_y, A4_DIRECTION_DOWN, 0, goto_best_priority);
            } else if (dy<0 && subject->canMove(A4_DIRECTION_UP, slack, checkForBridges)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_y, A4_DIRECTION_UP, 0, goto_best_priority);
            } else if (dx>0 && subject->canMove(A4_DIRECTION_RIGHT, slack, checkForBridges)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_x, A4_DIRECTION_RIGHT, 0, goto_best_priority);
            } else if (dx<0 && subject->canMove(A4_DIRECTION_LEFT, slack, checkForBridges)!=INT_MAX) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_x, A4_DIRECTION_LEFT, 0, goto_best_priority);
            }
            if (m_pathfinding_targets[goto_best].m_action == A4CHARACTER_COMMAND_ATTACK) {
                // look if we can attack:
                A4Object *target = m_pathfinding_targets[goto_best].m_target;
                if (target!=0) {
                    if (pixelx1==m_pathfinding_targets[goto_best].m_x0)
                        return new A4CharacterCommand(A4CHARACTER_COMMAND_ATTACK, 0, A4_DIRECTION_RIGHT, target, 0, goto_best_priority);
                    if (pixelx0==m_pathfinding_targets[goto_best].m_x1)
                        return new A4CharacterCommand(A4CHARACTER_COMMAND_ATTACK, 0, A4_DIRECTION_LEFT, target, 0, goto_best_priority);
                    if (pixely1==m_pathfinding_targets[goto_best].m_y0)
                        return new A4CharacterCommand(A4CHARACTER_COMMAND_ATTACK, 0, A4_DIRECTION_DOWN, target, 0, goto_best_priority);
                    if (pixely0==m_pathfinding_targets[goto_best].m_y1)
                        return new A4CharacterCommand(A4CHARACTER_COMMAND_ATTACK, 0, A4_DIRECTION_UP, target, 0, goto_best_priority);
                }
            }
            // However, if there is no alternative, at least try to stand still , since we are so close!
            // This prevents other behaviors from taking over movement, if this is the highest priority target:
            return new A4CharacterCommand(A4CHARACTER_COMMAND_IDLE, 0, 0, 0, goto_best_priority);
/*
            if (dy>0) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_y, A4_DIRECTION_DOWN, 0, goto_best_priority);
            } else {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_WALK, abs_diff_y, A4_DIRECTION_UP, 0, goto_best_priority);
            }
 */
        } else {
            // we are at the goal, stay!
            if (m_pathfinding_targets[goto_best].m_action == A4CHARACTER_COMMAND_TAKE) {
                return new A4CharacterCommand(A4CHARACTER_COMMAND_TAKE, 0, 0, 0, goto_best_priority);
            }
            return new A4CharacterCommand(A4CHARACTER_COMMAND_IDLE, 0, 0, 0, goto_best_priority);
        }
    }
    return 0;
}


/*
 - Input:
   - navigation perception buffer
   - m_pathfinding_gotoTargets and m_pathfinding_fleeTargets
 - Output:
   - the next coordinates to go to (in tile coordinates)
 */
bool A4AI::pathFinding(A4Object *subject) {
    if (m_pathfinding_n_targets==0) return false;

    int otx;
    int oty;
    // compute the origin tile cordinates (from the center of the top-left tile of the sprite):
    int pw = subject->getPixelWidth();
    int ph = subject->getPixelHeight();
    int tw = pw/tileWidth;
    int th = ph/tileHeight;
    if (pw>tileWidth) {
        otx = (subject->getX() + tileWidth/2)/tileWidth;
    } else {
        otx = (subject->getX() + subject->getPixelWidth()/2)/tileWidth;
    }
    if (ph>tileHeight) {
        oty = (subject->getY() + tileHeight/2)/tileHeight;
    } else {
        oty = (subject->getY() + subject->getPixelHeight()/2)/tileHeight;
    }
    
    if (otx<m_navigationBuffer_x || otx>m_navigationBuffer_x+m_navigationBuffer_size ||
        oty<m_navigationBuffer_y || oty>m_navigationBuffer_y+m_navigationBuffer_size) {
        output_debug_message("A4AI: character is out of the navigation buffer!!!\n");
        return false;
    }

    int start = (otx - m_navigationBuffer_x) + (oty - m_navigationBuffer_y)*m_navigationBuffer_size;
    int openInsertPosition = 0;
    int openRemovePosition = 0;
    int current,currentx,currenty;
    int next;
    int i;
    int bestScore = INT_MIN;
    int bestPriority = INT_MIN;
    int bestTarget = start;
    int score = 0, priority = 0;
    int size_sq = m_navigationBuffer_size*m_navigationBuffer_size;
//    int debug_order[size_sq];

    // initialize open/closed lists:
    if (m_pathfinding_open==0) {
        m_pathfinding_open = new int[size_sq];
        m_pathfinding_closed = new int[size_sq];
    }
    for(i = 0;i<m_navigationBuffer_size*m_navigationBuffer_size;i++) {
        m_pathfinding_closed[i] = -1;
//        debug_order[i] = 0;
    }
    m_pathfinding_open[openInsertPosition++] = start;
    m_pathfinding_closed[start] = start;
    openInsertPosition = openInsertPosition % size_sq;

    
    // pathfinding:
    m_pathFinding_iterations = 0;
    while(openRemovePosition!=openInsertPosition) {
        current = m_pathfinding_open[openRemovePosition];
//        debug_order[current] = m_pathFinding_iterations + 1;
        currentx = (current % m_navigationBuffer_size) + m_navigationBuffer_x;
        currenty = (current / m_navigationBuffer_size) + m_navigationBuffer_y;
        openRemovePosition++;
        openRemovePosition = openRemovePosition % size_sq;

        pathFindingScore(currentx,currenty, score, priority, subject);
        if (priority > bestPriority ||
            (priority == bestPriority && score > bestScore)) {
            bestScore = score;
            bestTarget = current;
            bestPriority = priority;
        }

        // neighbors:
        if (m_navigationBuffer[current]!=NAVIGATION_BUFFER_BRIDGE) {
            // LEFT:
            if (currentx > m_navigationBuffer_x) {
                next = current-1;
                bool canWalk = true;
                for(int i = 0;i<th;i++) {
                    if (m_navigationBuffer[next+i*m_navigationBuffer_size]==NAVIGATION_BUFFER_NOT_WALKABLE) {
                        canWalk = false;
                        break;
                    }
                }
                if (canWalk &&
                    m_pathfinding_closed[next]==-1) {
                    m_pathfinding_open[openInsertPosition++] = next;
                    openInsertPosition = openInsertPosition % size_sq;
                    m_pathfinding_closed[next] = current;   // store the parent
                }
            }
            // RIGHT:
            if (currentx < m_navigationBuffer_x+m_navigationBuffer_size - tw) {
                next = current+1;
                bool canWalk = true;
                for(int i = 0;i<th;i++) {
                    if (m_navigationBuffer[next+(tw-1)+i*m_navigationBuffer_size]==NAVIGATION_BUFFER_NOT_WALKABLE) {
                        canWalk = false;
                        break;
                    }
                }
                if (canWalk &&
                    m_pathfinding_closed[next]==-1) {
                    m_pathfinding_open[openInsertPosition++] = next;
                    openInsertPosition = openInsertPosition % size_sq;
                    m_pathfinding_closed[next] = current;   // store the parent
                }
            }
            // UP:
            if (currenty > m_navigationBuffer_y) {
                next = current-m_navigationBuffer_size;
                bool canWalk = true;
                for(int i = 0;i<th;i++) {
                    if (m_navigationBuffer[next+i]==NAVIGATION_BUFFER_NOT_WALKABLE) {
                        canWalk = false;
                        break;
                    }
                }
                if (canWalk &&
                    m_pathfinding_closed[next]==-1) {
                    m_pathfinding_open[openInsertPosition++] = next;
                    openInsertPosition = openInsertPosition % size_sq;
                    m_pathfinding_closed[next] = current;   // store the parent
                }
            }
            // DOWN:
            if (currenty < m_navigationBuffer_y+m_navigationBuffer_size - th) {
                next = current+m_navigationBuffer_size;
                bool canWalk = true;
                for(int i = 0;i<th;i++) {
                    if (m_navigationBuffer[next+(th-1)*m_navigationBuffer_size+i]==NAVIGATION_BUFFER_NOT_WALKABLE) {
                        canWalk = false;
                        break;
                    }
                }
                if (canWalk &&
                    m_pathfinding_closed[next]==-1) {
                    m_pathfinding_open[openInsertPosition++] = next;
                    openInsertPosition = openInsertPosition % size_sq;
                    m_pathfinding_closed[next] = current;   // store the parent
                }
            }
        }
        m_pathFinding_iterations++;
    }
    
    /*
    output_debug_message("%i pathfinding done from (%i,%i) : (%i,%i): final cost %i\n",m_cycle, subject->getX(), subject->getY(), otx, oty, bestScore);
    output_debug_message("Targets:");
    for(int i = 0;i<m_pathfinding_n_targets;i++) {
        output_debug_message("  (%i,%i)-(%i,%i) : %s\n",m_pathfinding_targets[i].m_x0,m_pathfinding_targets[i].m_y0,
                                                        m_pathfinding_targets[i].m_x1,m_pathfinding_targets[i].m_y1,
                                                        (m_pathfinding_targets[i].m_target == 0 ? "-":m_pathfinding_targets[i].m_target->getName()->get()));
    }
    */
    // reconstruct the path:
    current = bestTarget;
//    int px = (current % m_navigationBuffer_size) + m_navigationBuffer_x;
//    int py = (current / m_navigationBuffer_size) + m_navigationBuffer_y;
//    output_debug_message("Path:");
//    output_debug_message("  (%i,%i) : (%i,%i)\n", px*tileWidth, py*tileHeight, px, py);
    while(m_pathfinding_closed[current]!=-1 && m_pathfinding_closed[current]!=start) {
        current = m_pathfinding_closed[current];
//        {
            // check if this step is already close enough to be the target:
//            int px = (current % m_navigationBuffer_size) + m_navigationBuffer_x;
//            int py = (current / m_navigationBuffer_size) + m_navigationBuffer_y;
//            output_debug_message("  (%i,%i) : (%i,%i)\n", px*tileWidth, py*tileHeight, px, py);
//            int dx = abs(subject->getX() - px*tileWidth);
//            int dy = abs(subject->getY() - py*tileHeight);
            /*
            if (dx<tileWidth && dy<tileHeight) {
                m_pathfinding_result_x = px;
                m_pathfinding_result_y = py;
                m_pathfinding_result_priority = bestPriority;
                m_pathFinding_lastUpdated = m_cycle;
                return true;
            }
            */
//        }
//        current = m_pathfinding_closed[current];
    }
    /*
    output_debug_message("Order:\n");
    for(int i = 0;i<m_navigationBuffer_size;i++) {
        for(int j = 0;j<m_navigationBuffer_size;j++) {
            if (debug_order[j+i*m_navigationBuffer_size]==0) {
                output_debug_message(" - ",debug_order[j+i*m_navigationBuffer_size]);
            } else {
                output_debug_message("%02i ",debug_order[j+i*m_navigationBuffer_size]);
            }
        }
        output_debug_message("\n");
    }
    
    output_debug_message("Cost:\n");
    for(int i = 0;i<m_navigationBuffer_size;i++) {
        for(int j = 0;j<m_navigationBuffer_size;j++) {
            int tmp1, tmp2;
            pathFindingScore(j+m_navigationBuffer_x,i+m_navigationBuffer_y,tmp1,tmp2);
            output_debug_message("%02i ",-tmp1);
        }
        output_debug_message("\n");
    }
    
    output_debug_message("Walls:\n");
    for(int i = 0;i<m_navigationBuffer_size;i++) {
        for(int j = 0;j<m_navigationBuffer_size;j++) {
            output_debug_message("%02i ",m_navigationBuffer[j+i*m_navigationBuffer_size]);
        }
        output_debug_message("\n");
    }
    */
    
    m_pathfinding_result_x = (current % m_navigationBuffer_size) + m_navigationBuffer_x;
    m_pathfinding_result_y = (current / m_navigationBuffer_size) + m_navigationBuffer_y;
    m_pathfinding_result_priority = bestPriority;

    m_pathFinding_lastUpdated = m_cycle;

    return true;
}


void A4AI::pathFindingScore(int character_x0, int character_y0, int &out_score, int &priority, A4Object *subject)
{
    int bestGotoScorePriority = -1;
    int bestGotoScore = m_navigationBuffer_size+m_navigationBuffer_size;
    int bestFleeScorePriority = -1;
    int bestFleeScore = 0;
    int score = 0;
    int dx,dy;
    int character_x1 = character_x0 + (subject->getPixelWidth()/tileWidth)-1;
    int character_y1 = character_y0 + (subject->getPixelHeight()/tileHeight)-1;
    int x0,y0,x1,y1;
    
    for(int i = 0;i<m_pathfinding_n_targets;i++) {
        if (m_pathfinding_targets[i].m_flee) {
            if (m_pathfinding_targets[i].m_priority >= bestFleeScorePriority) {
                dx = dy = 0;
                x0 = m_pathfinding_targets[i].m_x0/tileWidth;
                y0 = m_pathfinding_targets[i].m_y0/tileHeight;
                x1 = (m_pathfinding_targets[i].m_x1-1)/tileWidth;
                y1 = (m_pathfinding_targets[i].m_y1-1)/tileHeight;
                if (character_x1<x0) dx = x0 - character_x1;
                if (character_x0>x1) dx = character_x0 - x1;
                if (character_y1<y0) dy = y0 - character_y1;
                if (character_y0>y1) dy = character_y0 - y1;
                
                score = dx + dy;
                if (m_pathfinding_targets[i].m_priority == bestFleeScorePriority) {
                    bestFleeScore += score; // for "flee", if it's the same priority, just add the scores (to "flee" from all of them, and not just one)
                } else {
                    if (score > bestFleeScore) {
                        bestFleeScore = score;
                        bestFleeScorePriority = m_pathfinding_targets[i].m_priority;
                    }
                }
            }
        } else {
            if (m_pathfinding_targets[i].m_priority >= bestGotoScorePriority) {
                dx = dy = 0;
                x0 = m_pathfinding_targets[i].m_x0/tileWidth;
                y0 = m_pathfinding_targets[i].m_y0/tileHeight;
                x1 = (m_pathfinding_targets[i].m_x1-1)/tileWidth;
                y1 = (m_pathfinding_targets[i].m_y1-1)/tileHeight;
                if (character_x1<x0) dx = x0 - character_x1;
                if (character_x0>x1) dx = character_x0 - x1;
                if (character_y1<y0) dy = y0 - character_y1;
                if (character_y0>y1) dy = character_y0 - y1;
                score = dx + dy;
                if (score < bestGotoScore) {
                    bestGotoScore = score;
                    bestGotoScorePriority = m_pathfinding_targets[i].m_priority;
                }
            }
        }
    }

    if (bestGotoScorePriority > bestFleeScorePriority) {
        out_score = - bestGotoScore;
        priority = bestGotoScorePriority;
    } else if (bestGotoScorePriority < bestFleeScorePriority) {
        out_score = bestFleeScore;
        priority = bestFleeScorePriority;
    } else {
        out_score = bestFleeScore - bestGotoScore;
        priority = bestGotoScorePriority;
    }
}
