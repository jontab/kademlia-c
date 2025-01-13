# kademlia-c

Kademlia is an ingenious distributed hash-table (DHT) protocol that makes use of the XOR metric to measure "distance" between peers. Keys and values are given to the K-closest peers to the hash (256-bits in our case) of the given key. Because of the XOR metric, we can find the K-closest peers in time proportional to the log of the size of the network. That means in a network with a million peers, it might only take about 20 steps to find those responsible for a given key.

This is a work in-progress. I am writing it in C because I like the challenge.

# External Dependencies

The two external dependencies that cannot be resolved via the `git submodule` command below are `libuv` and `openssl`. The first is a cross-platform library used for asynchronous I/O operations. The second is a library used for the SHA256 hash function.

```sh
jonab@MacBookPro kademlia-c % brew install libuv
jonab@MacBookPro kademlia-c % brew install openssl
```

```sh
jonab@Ubuntu kademlia-c % sudo apt install libuv1-dev
jonab@Ubuntu kademlia-c % sudo apt install [OPENSSL LIB NAME, libssl-dev?]
```

# Building

We use standard CMake to build the library and testing suite. For example, these are the commands I run to test locally.

```sh
jonab@MacBookPro kademlia-c % cmake -S . -B ../kademlia-build
jonab@MacBookPro kademlia-c % make -C ../kademlia-build
jonab@MacBookPro kademlia-c % ../kademlia-build/kademlia-tests
```

You might need to run this command beforehand to pull our submodules.

```sh
jonab@MacBookPro kademlia-c % git submodule update --init --recursive --depth=1
```

# Milestones

- [x] Uint256, Contact, OrderedDict, and Bucket.
- [x] Routing table (without protocol for replacing old peer).
- [x] Contact heap.
- [x] Crawler and contract crawler.
- [x] Protocol.
- [x] Client.
- [ ] Client refreshing.

# Stretch goals

- [ ] More efficient OrderedDict.
