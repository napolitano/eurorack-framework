/** @file fixed_slot.cpp @brief Implements marker-last fixed slots.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * @details Writes payload bytes before the validity marker so interrupted writes remain invalid.*/
#include <eurorack/storage/fixed_slot.hpp>
namespace eurorack::storage {
eurorack::io::IoResult FixedSlot::store(const std::uint8_t* p,std::size_t n) noexcept{if((p==nullptr&&n>0U)||n!=config_.payloadSize)return eurorack::io::IoResult::InvalidArgument;std::uint8_t invalid=0U;auto r=storage_.write(config_.address,&invalid,1U);if(r!=eurorack::io::IoResult::Success)return r;if(n>0U){r=storage_.write(config_.address+1U,p,n);if(r!=eurorack::io::IoResult::Success)return r;}r=storage_.write(config_.address,&config_.marker,1U);if(r!=eurorack::io::IoResult::Success)return r;return storage_.commit();}
eurorack::io::IoResult FixedSlot::load(std::uint8_t* p,std::size_t c) const noexcept{if((p==nullptr&&c>0U)||c<config_.payloadSize)return eurorack::io::IoResult::InvalidArgument;std::uint8_t m=0U;auto r=storage_.read(config_.address,&m,1U);if(r!=eurorack::io::IoResult::Success)return r;if(m!=config_.marker)return eurorack::io::IoResult::Busy;return storage_.read(config_.address+1U,p,config_.payloadSize);}
eurorack::io::IoResult FixedSlot::clear() noexcept{std::uint8_t invalid=0U;auto r=storage_.write(config_.address,&invalid,1U);if(r!=eurorack::io::IoResult::Success)return r;return storage_.commit();}
bool FixedSlot::valid() const noexcept{std::uint8_t m=0U;return storage_.read(config_.address,&m,1U)==eurorack::io::IoResult::Success&&m==config_.marker;}
} // namespace eurorack::storage
