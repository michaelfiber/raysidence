package mike

import "encoding/json"

func Map[K any, V any](items []K, cb func(item K) V) []V {
	var res []V

	for _, i := range items {
		res = append(res, cb(i))
	}

	return res
}

func Filter[K any](items []K, cb func(item K) bool) []K {
	var res []K

	for _, i := range items {
		if cb(i) {
			res = append(res, i)
		}
	}

	return res
}

func ToJson(v any) (string, error) {
	data, err := json.Marshal(v)
	return string(data), err
}

func FromJson(v any, dataStr string) error {
	return json.Unmarshal([]byte(dataStr), v)
}
