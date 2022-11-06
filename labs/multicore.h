#pragma once
#include <atomic>
//
// INVARIANT: w_deleted_count <= w_deleting_count <= w_cached_read_count <= shared_read_count <= r_reading_count <= r_cached_write_count <= shared_write_count <= w_writing_count <= w_deleted_count + MAX_SIZE
//
// INVARIANT:      w_writing_count      - w_deleted_count     <= MAX_SIZE
// =========>      w_writing_count      - w_cached_read_count <= MAX_SIZE
// =========>      shared_write_count   - w_cached_read_count <= MAX_SIZE
// =========>      shared_write_count   - shared_read_count   <= MAX_SIZE
//
//
// INVARIANT: 0 <= r_cached_write_count - r_reading_count
// =========> 0 <= r_cached_write_count - shared_read_count
// =========> 0 <= shared_write_count   - shared_read_count
//
//
// THEOREM: =========> 0 <= shared_write_count   - shared_read_count   <= MAX_SIZE
//





//
//
// Channel/Queue:
//
// Shared between Producer and Consumer
//
struct channel_t{
  public:

    //insert your code here
    std::atomic<size_t> shared_read_count;
    std::atomic<size_t> shared_write_count;

  public:

    //
    // Intialize
    //
    channel_t(){

      // insert your code here
      shared_read_count = 0;
      shared_write_count = 0;

    }
};


//
// Producer's (not shared)
//
struct writeport_t{
public:
    //insert your code here
  size_t w_deleted_count;
  size_t w_deleting_count;
  size_t w_cached_read_count;
  size_t w_writing_count;
  size_t MAX_SIZE;

public:

  //
  // Intialize
  //
  writeport_t(size_t tsize)
  {
    //insert code here
    w_deleted_count = 0;
    w_deleting_count = 0;
    w_cached_read_count = 0;
    w_writing_count = 0;
    MAX_SIZE = tsize;

  }

public:

  //
  // no of entries available to write
  //
  // helper function for write_canreserve
  //
  size_t write_reservesize(){

    // insert your code here
    // tsize - number of entries 
    return MAX_SIZE - (w_writing_count - w_deleted_count);
  }

  //
  // Can write 'n' entries?
  //
  bool write_canreserve(size_t n){

    // insert your code here
    return (write_reservesize() >=n);
  }

  //
  // Reserve 'n' entries for write
  //
  size_t write_reserve(size_t n){
    // insert your code here
    size_t curr = (w_writing_count % MAX_SIZE);
    w_writing_count = w_writing_count + n; //space reserved for n things
    return curr;
  }

  //
  // Commit
  //
  // Read/Write shared memory data structure
  //
  void write_release(channel_t& ch){

    // insert your code here
    ch.shared_write_count = w_writing_count; // done atomically

  }




public:

  //
  //
  // Read/Write shared memory data structure
  //
  void read_acquire(channel_t& ch){

    //insert your code here
    w_cached_read_count = ch.shared_read_count;

  }




  //
  // No of entires available to delete
  //
  size_t delete_reservesize(){
    //insert your code here

    return (w_cached_read_count - w_deleted_count);
  }

  //
  // Can delete 'n' entires?
  //
  bool delete_canreserve(size_t n){
    //insert your code here
    return (delete_reservesize() >=n);
  }

  //
  // Reserve 'n' entries for deletion
  //
  size_t delete_reserve(size_t n){
    //insert your code here
    w_deleting_count = w_deleted_count + n;
    return (w_deleted_count % MAX_SIZE);
  }


  //
  // Update the state, if any.
  //
  void delete_release(){
    //insert your code here
    w_deleted_count = w_deleting_count;
  }


};


//
// Consumer's (not shared)
//
//
struct readport_t{
public:

  //insert your code here
  size_t r_reading_count;
  size_t r_cached_write_count;
  size_t MAX_SIZE;

public:
  //
  // Initialize
  //
  readport_t(size_t tsize)
  {

    r_reading_count = 0;
    r_cached_write_count =0;
    MAX_SIZE = tsize;
    //insert your code here

  }
  public:

  //
  // Read/Write shared memory data structure
  //
  void write_acquire(channel_t& ch){

    //insert your code here
    r_cached_write_count = ch.shared_write_count;

  }

  //
  // no of entries available to read
  //
  size_t read_reservesize(){

    //insert your code here
    return r_cached_write_count - r_reading_count;
  }

  //
  // Can Read 'n' entires?
  //
  bool read_canreserve(size_t n){

    //insert your code here
    return (read_reservesize() >=n);
  }

  //
  // Reserve 'n' entires to be read
  //
  size_t read_reserve(size_t n){

    //insert your code here
    size_t curr = (r_reading_count % MAX_SIZE);
    r_reading_count = r_reading_count + n; //space reserved for n things
    return curr;
  }

  //
  // Read/write shared memory data structure
  //
  void read_release(channel_t& ch){

    //insert your code here
    ch.shared_read_count = r_reading_count;

  }

};


