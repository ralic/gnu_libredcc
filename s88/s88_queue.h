/* 
 *  Copyright 2014 Andre Gruning <libredcc@email.de>
 *
 * This file is part of LibreDCC
 *
 * LibreDCC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LibreDCC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LibreDCC.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef S88_QUEUE
#define S88_QUEUE 1

#include "s88_types.h"



/*! \file
  \todo It would actually be good to implement the queue as a class -- we
  could then use it a lot of times in a lot of code, and C++ is not
  that much slower than plain C if used carefully and we could use templates! */

//! is there an element in the queue?
uint8_t has_reading();

//! push an element onto the queue.
void queue_reading(const reading_t reading);

//! pop an element from the queue.
reading_t dequeue_reading();




#endif
