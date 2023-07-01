# 캐시 구현 예제
std::list와 std::unordered_map을 통해 기본적인 LRU 캐시를 구현.

## LRU 캐시
양방향 list에 최근에 사용한 데이터를 head에 위치 시켜며 캐시의 저장 공간이 가득 찼을 땐 가장 마지막에 사용된 데이터가 삭제되는 구조.
std::unordered_map을 사용하는 이유는 캐시의 용량이 커졌을 때(list의 item들이 많을 때) 탐색을 줄이기 위해서 사용함.

캐시의 용량은 저장된 데이터의 수로 할지 할당 메모리로 할지 선택할 수 있도록 item의 크기 계산자를 입력해줘야 함.
- 참고
  - [DefaultBruteforceLruCacheItemSizeCalc](https://github.com/baejun-k/CacheExample/blob/master/LruCache/cache/BruteforceLruCache.h#L25), 
  - [DefaultBruteforceLruCacheItemMemSizeCalc](https://github.com/baejun-k/CacheExample/blob/master/LruCache/cache/BruteforceLruCache.h#L34)

메모리 크기를 둔 이유는 일 하다보니까 사용 메모리 제한을 둬야할 경우가 필요해서 추가 함.
