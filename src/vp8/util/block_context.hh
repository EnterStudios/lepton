#ifndef _BLOCK_CONTEXT_HH_
#define _BLOCK_CONTEXT_HH_


struct NeighborSummary {
    uint8_t nonzeros_and_edge_pixels[16];
    uint8_t num_nonzeros() const {
        return nonzeros_and_edge_pixels[0];
    }
    void set_num_nonzeros(uint8_t nz) {
        nonzeros_and_edge_pixels[0] = nz;
    }
    uint8_t horizontal(int index) const {
        return nonzeros_and_edge_pixels[index + 8];
    }
    uint8_t vertical(int index) const {
        return nonzeros_and_edge_pixels[index == 0 ? 8 : index];
    }
    void set_horizontal(int * data) {
        for (int i = 0; i < 8 ; ++i) {
            nonzeros_and_edge_pixels[i + 8] = (uint8_t)std::max(std::min(data[i] + 128, 256), 0);
        }
        assert(data[8] == (uint8_t)std::max(std::min(data[i] + 128, 256), 0));
    }
    void set_vertical(int * data) {
        for (int i = 1; i < 8 ; ++i) {
            nonzeros_and_edge_pixels[i] = (uint8_t)std::max(std::min(data[i] + 128, 256), 0);
        }
    }
};

// in raytracing we usually find that having 32 bit offsets to pointers ends up being more
// efficient in our datastructures, since array offseting instructions are so fast.
template <class ABlock> struct MBlockContext {
    ABlock * cur;
    ABlock * above; //offset from cur; 0 for unavail
    std::vector<NeighborSummary>::iterator num_nonzeros_here;
    std::vector<NeighborSummary>::iterator num_nonzeros_above;
    MBlockContext<const AlignedBlock> copy() const {
        return {cur, above, num_nonzeros_here, num_nonzeros_above};
    }
    constexpr const ABlock& here() const {
        return cur[0];
    }
    constexpr const ABlock& left_unchecked() const {
        return cur[-1];
    }
    constexpr const ABlock& above_unchecked() const {
        return above[0];
    }
    constexpr const ABlock& above_left_unchecked() const {
        return above[-1];
    }
    static MBlockContext nil() {
        MBlockContext retval;
        memset(&retval, 0, sizeof(retval));
        retval.cur = nullptr;
        retval.above = nullptr;
        return retval;
    }
    bool isNil() {
        return cur == nullptr && above == nullptr;
    }
    ABlock& here() {
        return cur[0];
    }
    ABlock& left_unchecked() {
        return cur[-1];
    }
    ABlock& above_unchecked() {
        return above[0];
    }
    ABlock& above_left_unchecked() {
        return above[-1];
    }
    bool num_nonzeros_check(uint8_t nz7x7, ABlock& block) const{
        int nz = 0;
        for (int i = 1; i < 8; ++i) {
            for (int j = 1; j < 8; ++j) {
                if (block.coefficients_raster(i * 8 +j)) {
                    ++nz;
                }
            }
        }
        if (nz == nz7x7) {
            return true;
        }
        return false;
    }
    uint8_t nonzeros_above_7x7_unchecked() const{
        // too slow // assert(num_nonzeros_check(*num_nonzeros_above, above_unchecked()));
        return num_nonzeros_above->num_nonzeros();
    }
    uint8_t nonzeros_left_7x7_unchecked() const{
        std::vector<NeighborSummary>::iterator  tmp = num_nonzeros_here;
        --tmp;
        // too slow // assert(num_nonzeros_check(*tmp, left_unchecked()));
        return tmp->num_nonzeros();
    }
};
typedef MBlockContext<AlignedBlock> BlockContext;
typedef MBlockContext<const AlignedBlock> ConstBlockContext;

struct VContext {
    BlockContext context;
    int y;
    VContext(): context(BlockContext::nil()), y(0) {}
};
struct KVContext {
    ConstBlockContext context;
    int y;
    KVContext(): context(ConstBlockContext::nil()), y(0) {}
};
#endif
