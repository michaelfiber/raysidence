package helper

func Filter[K any](s []K, f func(t K) bool) []K {
	var result []K

	for _, i := range s {
		if f(i) {
			result = append(result, i)
		}
	}

	return result
}

func Map[K any, L any](slice []K, mapper func(index int, item K) L) []L {
	var result []L

	for i, item := range slice {
		result = append(result, mapper(i, item))
	}

	return result
}

func FilteredMap[K any, L any](slice []K, mapper func(index int, item K) (L, bool)) []L {
	var result []L

	for index, item := range slice {
		if filteredItem, ok := mapper(index, item); ok {
			result = append(result, filteredItem)
		}
	}

	return result
}
