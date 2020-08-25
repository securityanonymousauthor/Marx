#ifndef MAPPED_MACHO_H
#define MAPPED_MACHO_H

#include <string>
#include <vector>
#include <cstdint>

#include "macho.h"

#include "memory.h"

/*!
 * \brief Class holding information about a memory-mapped ELF file.
 */
class MappedMachO : public Memory {
private:
    std::vector<char> _buffer;

    mach_header_64 *_header = nullptr;
    segment_command_64 *_seg = nullptr;
    section_64 *_text_sec = nullptr;

    uintptr_t _base = 0;
    size_t _size = 0;
    //uintptr_t _file_addr = 0;

    std::vector<uintptr_t> _text_sec_bases;
    std::vector<size_t> _text_sec_sizes;
    std::vector<uintptr_t> _text_sec_fileaddrs;
    

public:
    MappedMachO(const MappedMachO&) = delete;
    virtual void operator=(const MappedMachO&) = delete;

    MappedMachO(const std::string &elf_file);
    virtual const uint8_t *operator[](const uintptr_t index) const;

    /*!
     * \brief Returns the begin of the executable `LOAD` segment.
     * \return Returns a pointer to the segment's begin.
     */
    virtual uintptr_t get_load_begin() const {
        return _base;
    }

    /*!
     * \brief Returns the end of the executable `LOAD` segment.
     * \return Returns a pointer to the segment's end.
     */
    virtual uintptr_t get_load_end() const {
        return _base + _size;
    }
};

#endif // MAPPED_ELF_H
