select type, count(id)
from Pokemon
group by type
order by count(id), type;