#include "frida-gum.h"

#include <stdint.h>
#include <stdio.h>

static int __attribute__((noinline))
stalker_followed_once() {
  uint64_t lo = 0, hi = 0;

  // Canary value
  const uint64_t canary = (uint64_t)0xDEADBEEFCAFEBABE;

  __asm__ __volatile__ (
      "mov   x9, sp\n"

      // Store the canary value past the red zone where Frida backup registers
      // x16/x17
      "stur  %[can], [x9, #-0x88]\n"
      "stur  %[can], [x9, #-0x90]\n"

      // Trigger a Frida context switch
      "bl    1f\n"
      "b     2f\n"
      "1:\n"
      "ret\n"
      "2:\n"

      // Read the values past the red zone
      "ldur  %[hi], [x9, #-0x88]\n"
      "ldur  %[lo], [x9, #-0x90]\n"

      : [lo] "=&r" (lo), [hi] "=&r" (hi)
      : [can] "r" (canary)
      : "x9", "x30", "memory", "cc");

  return lo != canary || hi != canary;
}

int stalker_is_following_this_thread() {
  static const int ROUNDS = 9;

  int hits = 0;
  for (int i = 0; i != ROUNDS; i++)
    hits += stalker_followed_once();

  return hits == ROUNDS;
}


int main() {
  // Run the detection without the stalker
  int native_result = stalker_is_following_this_thread ();

  gum_init_embedded();
  GumStalker* stalker = gum_stalker_new();
  gum_stalker_follow_me(stalker, NULL, NULL);
  // Run the detection under the stalker
  int stalked_result = stalker_is_following_this_thread();
  gum_stalker_unfollow_me(stalker);

  printf("native  : %s\n", native_result ? "DETECTED" : "CLEAN");
  printf("stalked : %s\n", stalked_result ? "DETECTED" : "CLEAN");

  return 0;
}


