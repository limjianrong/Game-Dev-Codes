/*====================================================================================
All content (c) 2024 Digipen Institute of Technology Singapore. All rights reserved.
Filename:   collision_detection.cpp
Project:    The Great Migration
Author(s):  Lim Jian Rong <jianrong.lim@digipen.edu> (primary: 100 %)
            NAME <EMAIL@digipen.edu> (secondary: 00 %)
            NAME <EMAIL@digipen.edu> (secondary: 00 %)
            NAME <EMAIL@digipen.edu> (secondary: 00 %)

Contents:
    - test_rect_to_rect
        checks for AABB static vs static rectangular collision
        checks for AABB dynamic vs static rectangular collision
        checks for AABB dynamic vs dynamic rectangular collision
    
    - sweep and prune
        broad phase collision detection

    - post collision update
        resolves collisions and updates positions and velocities of entities
    - compare_AABB:operator()
        used as the sorting condition in std::sort to sort AABBs in 
        ascending order
====================================================================================*/

/*                                                                          includes
====================================================================================*/
#include "pch.hpp"

/*                                                              function definitions
====================================================================================*/

/*!*****************************************************************************
\brief test_rect_to_rect
        checks for AABB static vs static rectangular collision
        checks for AABB dynamic vs static rectangular collision
        checks for AABB dynamic vs dynamic rectangular collision

\param[in] entity1
    first entity to check for collision

\param[in] entity2
    second entity to check with for collision

\param[in] delta_time
    time taken for one game loop

\param[in/out] t_first
    time to first collision between two rectangles, t_first will be used to for collision
    resolution if there is collision

\return
    Returns true if there is collision, returns false otherwise
*******************************************************************************/
std::vector<RE_ecs::entity*> RE_physics::colliding_entities;

bool RE_physics::test_rect_to_rect(RE_ecs::entity& entity1, RE_ecs::entity& entity2, double delta_time, double& t_first) {

    REUTIL_math::vec2 velocity_a{};
    REUTIL_math::vec2 velocity_b{};

    RE_component::rigid_body_component& entity1_rigid_body = entity1.get_component<RE_component::rigid_body_component>();    

    
    RE_component::rigid_body_component& entity2_rigid_body = entity2.get_component<RE_component::rigid_body_component>();

        velocity_a = entity1_rigid_body.velocity;
        velocity_b = entity2_rigid_body.velocity;


    //time to first intersection along x-axis, y-axis, time to last point of contact along
    //x-axis, y-axis
    double t_first_x{}, t_last_x{}, t_first_y{}, t_last_y{};

    //time to last point of intersection between two rectangles
    double t_last{ delta_time };

    //calculate relative velocity by setting one of the velocities to zero
    REUTIL_math::vec2 relative_velocity = velocity_a - velocity_b;
    RE_component::AABB& entity1_bounding_box = entity1.get_component<RE_component::collision>().bounding_box;
    RE_component::AABB& entity2_bounding_box = entity2.get_component<RE_component::collision>().bounding_box;


        if ((entity1_bounding_box.min.x > entity2_bounding_box.max.x) && relative_velocity.x >= 0.f) {
            return false;
        }

        //if entity2 is to the right of entity2, and entity1 is not moving, or moving left, return false
        else if ((entity2_bounding_box.min.x > entity1_bounding_box.max.x) && relative_velocity.x <= 0.f) {
            return false;
        }

        //if entity1 is below entity2, and entity1 is not moving, or moving down, return false
        if ((entity1_bounding_box.max.y < entity2_bounding_box.min.y) && relative_velocity.y <= 0.f) {
            return false;
        }

        //if entity2 is on below of entity2, and entity1 is not moving, or moving up, return false
        else if ((entity2_bounding_box.max.y < entity1_bounding_box.min.y) && relative_velocity.y >= 0.f) {
            return false;
        }

    //if the code reaches here, it means that the rectangles are on path to collide with one another
    //we now need to calculate the time to collision

    //if relative velocity is towards the left
    if (relative_velocity.x <= 0) {

        //calculate time to intersection along x-axis
        if (entity2_bounding_box.max.x < entity1_bounding_box.min.x) {
            t_first_x = (entity2_bounding_box.max.x - entity1_bounding_box.min.x) / relative_velocity.x;
        }

        //calculate time to last intersection along x-axis
        if (entity2_bounding_box.min.x < entity1_bounding_box.max.x) {
            t_last_x = (entity2_bounding_box.min.x - entity1_bounding_box.max.x) / relative_velocity.x;
        }

        //if t_first_x > delta_time, collision will not occur within this frame, return false
        if (t_first_x > delta_time) {
            return false;
        }
    }


    //if relative velocity is towards the right
    if (relative_velocity.x >= 0) {
        //calculate time to intersection along x-axis
        if (entity2_bounding_box.min.x > entity1_bounding_box.max.x) {
            t_first_x = (entity2_bounding_box.min.x - entity1_bounding_box.max.x) / relative_velocity.x;
        }

        //calculate time to last intersection along x-axis
        if (entity2_bounding_box.max.x > entity1_bounding_box.min.x) {
            t_last_x = (entity2_bounding_box.max.x - entity1_bounding_box.min.x) / relative_velocity.x;
        }

        //if t_first_x > delta_time, collision will not occur within this frame, return false
        if (t_first_x > delta_time) {
            return false;
        }
    }

    //if relative velocity is downwards
    if (relative_velocity.y <= 0) {

        //calculate time to intersection along y-axis
        if (entity2_bounding_box.max.y < entity1_bounding_box.min.y) {
            t_first_y = (entity2_bounding_box.max.y - entity1_bounding_box.min.y) / relative_velocity.y;
        }

        //calculate time to last intersection along y-axis
        if (entity2_bounding_box.min.y < entity1_bounding_box.max.y) {
            t_last_y = (entity2_bounding_box.min.y - entity1_bounding_box.max.y) / relative_velocity.y;
        }

        //if t_first_y > delta_time, collision will not occur within this frame, return false
        if (t_first_y > delta_time) {
            return false;
        }
    }

    //if relative velocity is upwards
    if (relative_velocity.y >= 0) {

        //calculate time to intersection along y-axis
        if (entity2_bounding_box.min.y > entity1_bounding_box.max.y) {
            t_first_y = (entity2_bounding_box.min.y - entity1_bounding_box.max.y) / relative_velocity.y;
        }

        //calculate time to last intersection along y-axis
        if (entity2_bounding_box.max.y > entity1_bounding_box.min.y) {
            t_last_y = (entity2_bounding_box.max.y - entity1_bounding_box.min.y) / relative_velocity.y;
        }

        //if t_first_y > delta_time, collision will not occur within this frame, return false
        if (t_first_y > delta_time) {
            return false;
        }
    }

    //time to first intersection is the larger of t_first_y and t_first_x
    t_first = t_first_y < t_first_x ? t_first_x : t_first_y;

    //time to last intersection is the smaller of t_last_y and t_last_x
    t_last = t_last_y > t_last_x ? t_last_x : t_last_y;

    //if t_first > t_last or t_first > delta_time, return false
    if (t_first > t_last || t_first > delta_time) {
        return false;
    }

    //if t_first <0, return false
    if (t_first < 0.0) {
        return false;
    }

    return true;
}


/*!*****************************************************************************
\brief compare_AABB::operator()

\param[in] lhs
    entity used for comparison


\param[in] rhs
    entity to compare against

\return
    Returns true if minimum point on AABB of entity lhs has a lower value than
    the minimum point on AABB of entity rhs
*******************************************************************************/
bool RE_physics::compare_AABB::operator()(RE_ecs::entity& lhs, RE_ecs::entity& rhs) {

    //compare along x axis
    if (sort_axis == x_axis) { 
        return lhs.get_component<RE_component::collision>().bounding_box.min.x < rhs.get_component<RE_component::collision>().bounding_box.min.x;
    }

    //compare along y axis
    return lhs.get_component<RE_component::collision>().bounding_box.min.y < rhs.get_component<RE_component::collision>().bounding_box.min.y;
}

/*!*****************************************************************************
\brief sweep and prune broad phase collision detection to allow for more efficient
        collision detection rather than using brute force collision detection

\param[in] all_relevant_entities
    vector of all AABBs of entities that have rigid body and collision component


\param[in] delta_time
    time step

\return
    None
*******************************************************************************/
void RE_physics::sweep_and_prune(std::vector<RE_ecs::entity>& all_relevant_entities, double delta_time) {

    //clear all entities that are colliding in the previous loop
    colliding_entities.clear();

    //axis with the highest variance will be used as the axis to sort by when calling std::sort
    axis_to_sort_by = variance.x > variance.y ? x_axis : y_axis;

    //construct object that will be used as sorting condition
    compare_AABB compare_AABB(axis_to_sort_by);

    //sort AABBs in ascending order, along the sorting axis
        std::sort(all_relevant_entities.begin(), all_relevant_entities.end(), compare_AABB);

        //check for collision
        for (unsigned int i = 0; i < all_relevant_entities.size(); ++i) {

            if (!all_relevant_entities[i].has_component<RE_component::rigid_body_component>() || !all_relevant_entities[i].has_component<RE_component::collision>())
                continue;
            double t_first{}; //to be passed into test_rect_to_rect
            RE_component::rigid_body_component& lhs_entity_rigid_body_component = all_relevant_entities[i].get_component<RE_component::rigid_body_component>();
            RE_component::collision& lhs_entity_collision_component = all_relevant_entities[i].get_component<RE_component::collision>();
            RE_component::transform& lhs_entity_transform_component = all_relevant_entities[i].get_component<RE_component::transform>();
            lhs_entity_collision_component.minimum_t_first = delta_time; //reset minimum t_first to delta time

            //let u=i+1 to avoid duplicate checks (check A with B, and then checking B with A)
            for (unsigned int u = i + 1; u < all_relevant_entities.size(); ++u) {

                RE_component::rigid_body_component& rhs_entity_rigid_body_component = all_relevant_entities[u].get_component<RE_component::rigid_body_component>();
                RE_component::collision& rhs_entity_collision_component = all_relevant_entities[u].get_component<RE_component::collision>();
                RE_component::transform& rhs_entity_transform_component = all_relevant_entities[u].get_component<RE_component::transform>();
                rhs_entity_collision_component.minimum_t_first = delta_time;
                //early continue condition for AABBs sorting along x_axis in ascending order
                if (lhs_entity_collision_component.bounding_box.max.x < rhs_entity_collision_component.bounding_box.min.x && lhs_entity_rigid_body_component.velocity.x <= 0 && rhs_entity_rigid_body_component.velocity.x >= 0 && axis_to_sort_by==x_axis) {
                    continue;
                }

                //early continue condition for AABBs sorting along y_axis in ascending order
                else if (lhs_entity_collision_component.bounding_box.max.y < rhs_entity_collision_component.bounding_box.min.y && lhs_entity_rigid_body_component.velocity.y <= 0 && 
                    rhs_entity_rigid_body_component.velocity.y >= 0 && axis_to_sort_by == y_axis) {
                    continue;
                }

                //narrow phase detection
                if (test_rect_to_rect(all_relevant_entities[i], all_relevant_entities[u], delta_time, t_first)) {
                    lhs_entity_collision_component.objects_collided_with[all_relevant_entities[u].get_id()] = t_first;
                    rhs_entity_collision_component.objects_collided_with[all_relevant_entities[i].get_id()] = t_first;

                    //set boolean to true, entities have collided with each other
                    lhs_entity_collision_component.collided = true;
                    rhs_entity_collision_component.collided = true;
                   //each entity will keep track of the entities that it has collided with, 
                    //along with the time to first intersection with that entity
                    colliding_entities.push_back(RE_ecs::MANAGER->get_entity_from_id(all_relevant_entities[i].get_id()));
                    colliding_entities.push_back(RE_ecs::MANAGER->get_entity_from_id(all_relevant_entities[u].get_id()));
                    if (t_first == 0) {
                        //calculate the positive value of overlap between the two AABBs along x axis
                       // float positive_overlap_x{};
                        if (lhs_entity_transform_component.position.x < rhs_entity_transform_component.position.x) {
                            rhs_entity_collision_component.positive_overlap_x = lhs_entity_collision_component.positive_overlap_x = lhs_entity_collision_component.bounding_box.max.x - rhs_entity_collision_component.bounding_box.min.x;
                        }
                        else {
                            rhs_entity_collision_component.positive_overlap_x = lhs_entity_collision_component.positive_overlap_x  = rhs_entity_collision_component.bounding_box.max.x - lhs_entity_collision_component.bounding_box.min.x;
                        }

                        //calculate the positive value of overlap between the two AABBs along y axis
                       // float positive_overlap_y{};
                        if (lhs_entity_transform_component.position.y < rhs_entity_transform_component.position.y) {
                            rhs_entity_collision_component.positive_overlap_y = lhs_entity_collision_component.positive_overlap_y  = lhs_entity_collision_component.bounding_box.max.y - rhs_entity_collision_component.bounding_box.min.y;
                        }
                        else {
                            rhs_entity_collision_component.positive_overlap_y = lhs_entity_collision_component.positive_overlap_y = rhs_entity_collision_component.bounding_box.max.y - lhs_entity_collision_component.bounding_box.min.y;
                        }
                    }
                }
                else {

                    //if t_first > delt_time, entity will not collide with anymore objects that have a higher minimum point than itself
                    if (t_first > delta_time) {
                        break;
                    }
                }
            }
        }

        //find minimum t_first of each colliding entity
        for (unsigned int i = 0; i < colliding_entities.size(); ++i) {
            RE_ecs::entity& entity = *colliding_entities[i];

            RE_component::collision& entity_collision_component = entity.get_component<RE_component::collision>();

            if (entity_collision_component.objects_collided_with.size() != 0) {
                entity_collision_component.minimum_t_first = entity_collision_component.objects_collided_with.begin()->second;
                entity_collision_component.actual_entity_collided_with = entity_collision_component.objects_collided_with.begin()->first;

                for (std::map<int, double>::iterator it = entity_collision_component.objects_collided_with.begin(); it != entity_collision_component.objects_collided_with.end(); ++it) {
                    if (it->second < entity_collision_component.minimum_t_first) {
                        entity_collision_component.minimum_t_first = it->second;
                        entity_collision_component.actual_entity_collided_with = it->first;
                    }
                }
            }

        }
    
} //end of function definition of sweep_and_prune


/*!*****************************************************************************
\brief post_collision_update is responsible for changing the positions and velocities of
       every entity, depending on whether it has collided with another entity or not.

\param[in] all_relevant_entities
    vector of all AABBs of entities that have rigid body and collision component


\param[in] delta_time
    time step

\return
    None
*******************************************************************************/
void RE_physics::post_collision_update(std::vector<RE_ecs::entity>& all_relevant_entities, double delta_time) {
    for (RE_ecs::entity& entity : all_relevant_entities) {


        if (!entity.has_component<RE_component::rigid_body_component>() || !entity.has_component<RE_component::collision>()|| !entity.has_component<RE_component::rigid_body_component>())
            continue;

        if (!entity.has_component<RE_component::transform>())
            continue;

        RE_component::transform& entity_transform_component = entity.get_component<RE_component::transform>();
        RE_component::rigid_body_component& entity_rigid_body_component = entity.get_component<RE_component::rigid_body_component>();
        RE_component::collision& entity_collision_component = entity.get_component<RE_component::collision>();
        
        //entity has not collided with anything, increment its position by delta time
        if (!entity_collision_component.collided) {
            entity_transform_component.position += static_cast<float>(delta_time) * entity_rigid_body_component.velocity;

        }

        //entity has collided with another entity, determine if dynamic or static collision
        else {

            ////dynamic collision, increment entity by its minimum t_first
            if (entity_collision_component.minimum_t_first > 0) {
              //  std::cout << "t_first >0\n";
                entity_transform_component.position += static_cast<float>(entity_collision_component.minimum_t_first) * entity_rigid_body_component.velocity;

                // trying to move left, offset its x position to prevent constant collision
                if (entity_rigid_body_component.velocity.x < 0) {
                    entity_transform_component.position.x += offset_upon_collision;
                }

                //trying to move right, offset its x position to prevent constant collision
                else if (entity_rigid_body_component.velocity.x > 0) {
                    entity_transform_component.position.x -= offset_upon_collision;
                }

                //trying to move down, offset its y position to prevent constant collision
                if (entity_rigid_body_component.velocity.y < 0) {
                    entity_transform_component.position.y += offset_upon_collision;
                }

                //trying to move up, offset its y position to prevent constant collision
                else if (entity_rigid_body_component.velocity.y > 0) {
                    entity_transform_component.position.y -= offset_upon_collision;
                }

                //set velocity to 0
               // entity_rigid_body_component.velocity = { 0.f,0.f };

            }

            //if collided but t_first ==0, means that the entities are already colliding(static collision), enter th else if {}
             if (entity_collision_component.minimum_t_first == 0) {
             //    std::cout << "t_first == 0 for ID " << entity.get_id() << "\n";

                RE_ecs::entity& other_entity = *RE_ecs::MANAGER->get_entity_from_id(entity_collision_component.actual_entity_collided_with);

                if (!other_entity.has_component<RE_component::transform>())
                    continue;

                RE_component::transform& other_entity_transform_component = other_entity.get_component<RE_component::transform>();

                if (entity_collision_component.positive_overlap_x < entity_collision_component.positive_overlap_y) {

                    if (entity_transform_component.position.x < other_entity_transform_component.position.x) {
                        entity_transform_component.position.x -= 0.5f * entity_collision_component.positive_overlap_x;
                        entity_transform_component.position.x -= offset_upon_collision;
                    }
                    else {
                        entity_transform_component.position.x += 0.5f * entity_collision_component.positive_overlap_x;
                        entity_transform_component.position.x += offset_upon_collision;
                    }
                }

                //else, "push" them away from each other along the y axis
                else {
                    if (entity_transform_component.position.y < other_entity_transform_component.position.y) {
                        entity_transform_component.position.y -= 0.5f * entity_collision_component.positive_overlap_y;
                        entity_transform_component.position.y -= offset_upon_collision;
                    }

                    else {
                        entity_transform_component.position.y += 0.5f * entity_collision_component.positive_overlap_y;
                        entity_transform_component.position.y += offset_upon_collision;
                    }
                }
                entity_collision_component.positive_overlap_x = 0.f;
                entity_collision_component.positive_overlap_y = 0.f;
            //    //collision response -  set entity to stop moving / offset entities that are overlapping
            //    entity_rigid_body_component.velocity = { 0.f,0.f };
              //  entity_transform_component.position += static_cast<float>(delta_time) * entity_rigid_body_component.velocity;
               
            }
        }

        //update the min and max points of the bounding box
        RE_component::AABB& entity_aabb = entity_collision_component.bounding_box;
        entity_aabb.min = entity_transform_component.position - entity_collision_component.bounding_box_size*0.5f;
        entity_aabb.max = entity_transform_component.position + entity_collision_component.bounding_box_size*0.5f;


        if(!entity_collision_component.objects_collided_with.empty())
        //clear all the entity IDs that the entity has collided with in the current game loop
        entity_collision_component.objects_collided_with.clear();
    }
}



