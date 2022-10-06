## Octree Iteration pseudocode
it(ray, box_origin, box_size):
    near, far = vec3(0)
    near, far = AABB_intersection(ray, box_origin, box_size)

    old_box_origin = box_origin
    it_box_size = box_size / 2
    octant, new_center_delta = get_octant(near - box_center)
    it_box_center = box_center + new_center_delta * (it_box_size / 2)
    
    base_index = 0
    curr_octree_node = get_node(base_index, octant)
    
    for i in range(20):
        if curr_octree_node.type == FULL:
            return HIT
        elif curr_octree_node.type == EMPTY:
            return NO_HIT
        else:
            // Go down 1 level
            near, far = AABB_intersect(ray, it_box_center, it_box_size)
            octant, new_center_delta = get_octant(near - it_box_center)
            it_box_size = it_box_size / 2
            it_box_center = it_box_center + new_center_delta * (it_box_size / 2)
                        
            curr_octree_node = get_node(curr_octree_node, octant)
 
        
   
   
Escturura de OCtrees alternativa:
 2 Texeles
 Nodos hijos vacios marcados por un indice de 0 en el padre; y llenos marcados por un 1
 
