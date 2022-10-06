#ifndef VOLUMETRIC_OCTREE_H_
#define VOLUMETRIC_OCTREE_H_

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <cassert>

namespace OCTREE {
    enum eVoxelType : uint32_t {
        FULL_VOXEL = 0,
        EMPTY_VOXEL,
        MIXED_VOXEL,
        TBD_VOXEL
    };

    struct sVoxel {
        eVoxelType type;
        uint32_t  childs[8];
        uint32_t padding[3]; // Padding for texel aligment
    };


    struct sVolumeOctree {
        uint32_t raw_size = 0;
        uint32_t element_count = 0;
        uint32_t level_count = 0;

        sVoxel *nodes;

        inline uint32_t get_size_on_pixels() const {
            return element_count * 3; // One voxel = 3 texels
        }
    };

    struct sRawVolume {
        uint8_t *raw_volume;
        uint16_t width = 0;
        uint16_t heigth = 0;
        uint16_t depth = 0;

        uint8_t density_threshold = 15;

        inline eVoxelType get_pixel_state(const uint16_t x,
                                          const uint16_t y,
                                          const uint16_t z) const {
            uint8_t value = raw_volume[(x + y * width + z * heigth * width)];

            return (value > density_threshold) ? FULL_VOXEL : EMPTY_VOXEL;
        }

        inline uint8_t get_pixel(const uint16_t x,
                                 const uint16_t y,
                                 const uint16_t z) const {
            return raw_volume[(x + y * width + z * heigth * width)];
        }

    };


// Given a level and a position, gets the indices of the children
// plus a padding
    inline void fill_children_of_voxel(const uint32_t curr_level,
                                       const uint32_t x,
                                       const uint32_t y,
                                       const uint32_t z,
                                       const uint32_t padding,
                                       uint32_t *children) {
        uint32_t prev_width = pow(2, curr_level+1);

        uint32_t prev_x = x * 2;
        uint32_t prev_y = y * 2;
        uint32_t prev_z = z * 2;

        for(uint32_t z_delta = 0; z_delta < 2; z_delta++) {
            for(uint32_t y_delta = 0; y_delta < 2; y_delta++) {
                for(uint32_t x_delta = 0; x_delta < 2; x_delta++) {
                    uint32_t child_index = (x_delta + prev_x) + (y_delta + prev_y) * prev_width;
                    child_index += (z_delta + prev_z) * (prev_width * prev_width);
                    children[x_delta + y_delta * 2 + z_delta * 4] = (child_index + padding);
                }
            }
        }

    }


#define IS_POWER_OF_TWO(x) ((x & (x - 1)) == 0)


// Generates a volumetric octree with the given volumetric texture
    inline void octree_generation(const sRawVolume &raw_vol,
                                  sVolumeOctree *result) {
        assert(raw_vol.width == raw_vol.depth &&
               raw_vol.width == raw_vol.heigth &&
               IS_POWER_OF_TWO(raw_vol.width) &&
               "Error while generating octree, non square and power of two text");


        // Calculate the memmry chunk of the whle tree
        uint32_t octree_size = 0;
        uint32_t size = raw_vol.width;
        uint32_t level_count = 0;
        while(size >= 1) {
            octree_size += size*size*size;
            size /= 2;
            level_count++;
        }
        result->element_count = octree_size;
        result->raw_size = sizeof(sVoxel) * octree_size;
        result->level_count = level_count;
        sVoxel *octree = (sVoxel*) malloc(result->raw_size);

        // Populate the base of the octree
        const uint32_t base_size = raw_vol.width;
        const uint32_t base_area_size = raw_vol.width * raw_vol.width * raw_vol.width;
        const uint32_t base_index = octree_size - base_area_size;

        for(uint32_t i = 0; i < base_area_size; i++) {
            const uint32_t x = i % base_size;
            const uint32_t y = (i / base_size) % base_size;
            const uint32_t z = i / (base_size * base_size);

            octree[base_index + i].type = raw_vol.get_pixel_state(x,
                                                                  y,
                                                                  z);
        }

        // Generate all the layers octree bottom-to-top
        uint32_t current_level = level_count -2;
        uint32_t layer_size = base_size;
        uint32_t prev_layer_start = base_index;

        while(current_level >= 1) {
            layer_size /= 2;
            uint32_t layer_area = layer_size * layer_size * layer_size;
            // Get the starting index for the current level on the octree
            uint32_t layer_start = prev_layer_start - layer_area;

            for(uint32_t i = 0; i < layer_area; i++) {
                // For each voxel on the current level, read the children (the preovius level)
                // and set the current voxel type accordingly
                const uint32_t x = i % layer_size;
                const uint32_t y = (i / layer_size) % layer_size;
                const uint32_t z = i / (layer_size * layer_size);

                uint32_t index = i + layer_start;
                eVoxelType curr_voxel = TBD_VOXEL;

                fill_children_of_voxel(current_level,
                                       x,
                                       y,
                                       z,
                                       prev_layer_start,
                                       octree[index].childs);

                for(uint8_t child_id = 0; child_id < 8; child_id++) {
                    eVoxelType child_state = octree[octree[index].childs[child_id]].type;

                    if (curr_voxel == TBD_VOXEL) {
                        curr_voxel = child_state;
                    }

                    if (child_state == MIXED_VOXEL || child_state != curr_voxel) {
                        curr_voxel = MIXED_VOXEL;
                        break;
                    }
                }

                octree[index].type = curr_voxel;
            }

            current_level--;
            prev_layer_start = layer_start;
        }

        // Compute last level (1)

        eVoxelType curr_voxel = TBD_VOXEL;

        fill_children_of_voxel(0,
                               0,
                               0,
                               0,
                               1,
                               octree[0].childs);

        for(uint8_t child_id = 0; child_id < 8; child_id++) {
            eVoxelType child_state = octree[octree[0].childs[child_id]].type;

            if (curr_voxel == TBD_VOXEL) {
                curr_voxel = child_state;
            }

            if (child_state == MIXED_VOXEL || child_state != curr_voxel) {
                curr_voxel = MIXED_VOXEL;
                break;
            }
        }

        octree[0].type = curr_voxel;

        result->nodes = octree;
    }


};

#endif // VOLUMETRIC_OCTREE_H_
