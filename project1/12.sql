select distinct p.name, p.type
from Pokemon p, CatchedPokemon c
where p.id = c.pid and c.level >= 30
order by p.name;