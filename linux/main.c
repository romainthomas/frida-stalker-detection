#include "frida-gum.h"

#include <stdint.h>
#include <stdio.h>

static int __attribute__((noinline))
stalker_followed_once() {
  uint64_t a = 0, b = 0, c = 0;

  // Canary value
  const uint64_t canary = 0xDEADBEEFCAFEBABEull;

  __asm__ __volatile__ (
      ".intel_syntax noprefix\n"
      // Store the canary value past the red zone where Frida backup registers
      "mov   QWORD PTR [rsp - 0x88], %V[can]\n"
      "mov   QWORD PTR [rsp - 0x90], %V[can]\n"
      "mov   QWORD PTR [rsp - 0x98], %V[can]\n"

      // Trigger a Frida context switch
      "lea   rcx, [rip + 1f]\n"
      "jmp   rcx\n"
      "1:\n"

      // Read the values past the red zone
      "mov   %V[a], QWORD PTR [rsp - 0x88]\n"
      "mov   %V[b], QWORD PTR [rsp - 0x90]\n"
      "mov   %V[c], QWORD PTR [rsp - 0x98]\n"
      ".att_syntax prefix\n"
      : [a] "=&r" (a), [b] "=&r" (b), [c] "=&r" (c)
      : [can] "r" (canary)
      : "rcx", "memory");

  return a != canary || b != canary || c != canary;
}


// check if the stalker is attached to this thread
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

