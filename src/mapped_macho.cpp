
#include "mapped_macho.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

/*!
 * \brief Constructs a new `MappedMachO` instance from a given ELF file.
 * \param elf_file The path to the ELF file which is to be mapped.
 *
 * If the file cannot be found or seems to be malformed, a `runtime_error`
 * exception is thrown.
 */
MappedMachO::MappedMachO(const string &macho_file) {
    ifstream file(macho_file.c_str(), ios::binary);
    if(!file) {
        throw runtime_error("Cannot open file " + macho_file + ".");
    }

    _buffer = vector<char>(istreambuf_iterator<char>(file),
                           istreambuf_iterator<char>());
    _header = reinterpret_cast<mach_header_64*>(_buffer.data());
    _seg = reinterpret_cast<segment_command_64*>(_buffer.data() +
                                              sizeof(mach_header_64));

                                            
    _text_sec_bases = vector<uintptr_t>();
    _text_sec_sizes = vector<size_t>();
    _text_sec_fileaddrs = vector<uintptr_t>();
    uintptr_t last_text_base = 0;
    size_t last_text_size = 0;
    bool found_text = false;
    size_t seg_sizes = 0;

    printf("_header->ncmds: %d\n", _header->ncmds);
    for(auto i = 0; i < _header->ncmds; ++i) {

        _seg = reinterpret_cast<segment_command_64*>(_buffer.data() +
                                            sizeof(mach_header_64) + 
                                            seg_sizes);
        if(_seg->cmd == LC_SEGMENT_64){
            printf("%s nsects: %d\n", _seg->segname, _seg->nsects);
            for(auto j = 0; j < _seg->nsects; ++j) {
                _text_sec = reinterpret_cast<section_64*>(_buffer.data() +
                                                sizeof(mach_header_64) + 
                                                seg_sizes + 
                                                sizeof(segment_command_64) + 
                                                j*sizeof(section_64)
                                                );
                //printf("_text_sec: %p\n", _text_sec);
                if (strcmp(_text_sec->sectname, "__text") == 0){
                    printf("found text section %s %s\n", _seg->segname, _text_sec->sectname);
                    _text_sec_bases.push_back(_text_sec->addr);
                    _text_sec_sizes.push_back(_text_sec->size);
                    _text_sec_fileaddrs.push_back(_text_sec->offset);
                    if (_base == 0 || _text_sec->addr < _base){
                        _base =  _text_sec->addr;
                    }
                    if(_text_sec->addr > last_text_base){
                        last_text_base = _text_sec->addr;
                        last_text_size = _text_sec->size;
                    }
                }                              
            }
        }
        seg_sizes += _seg->cmdsize;
    }
    if(!_text_sec_bases.size()) {
        throw runtime_error("Malformed input file " + macho_file + ".");
    }
    _size = last_text_base + last_text_size - _base;
    // FIXME: We rely on compilers a bit here, this can be generalized.
    /*
    for(auto i = 0; i < _header->ncmds; ++i) {

        const auto &current = _seg[i];
        if(strcmp(current.segname, "__TEXT") == 0){
            printf("found TEXT seg %d %d\n", current.nsects, current.fileoff);
            for(auto j = 0; j < current.nsects; ++j) {
                _text_sec = reinterpret_cast<section_64*>(_buffer.data() +
                                              sizeof(mach_header_64) + 
                                              (i+1)*sizeof(segment_command_64) + 
                                              j*sizeof(section_64)
                                              );
                if (strcmp(_text_sec->sectname, "__text") == 0){
                    printf("found text section\n");
                    found_text = true;
                    break;
                }                              
            }
            if(found_text){
                _base = _text_sec->addr;
                _size = _text_sec->size;
                _file_addr = _text_sec->offset;
                break;

            }
        }
        
    }
    if(!_size) {
        throw runtime_error("Malformed input file " + macho_file + ".");
    }
    */
}

/*!
 * \brief Implements indexing access, effectively accessing the memory lieing
 * at the given virtual address.
 * \param address (Virtual) address of memory to access.
 * \return A pointer to the memory requested, if it lies at the given virtual
 * address. `nullptr` else.
 */
const uint8_t *MappedMachO::operator[](const uintptr_t address) const {
    for(int i=0; i<_text_sec_bases.size(); i++){
        uintptr_t base = _text_sec_bases[i];
        size_t size = _text_sec_sizes[i];
        uintptr_t file_addr = _text_sec_fileaddrs[i];
        if(address >= base && address < base + size) {
            const uint8_t *data = reinterpret_cast<const uint8_t*>(_buffer.data());
            return data + address - base + file_addr;
        }
    }
    printf("addr(%p) out-of-range base\n", address);
    return nullptr;

    /*
    if(address < _base || address > _base + _size) {
        printf("addr(%p) out-of-range base(%p) size(%d)\n", address, _base, _size);
        return nullptr;
    }

    const uint8_t *data = reinterpret_cast<const uint8_t*>(_buffer.data());

    return data + address - _base + _file_addr;
    */
}
