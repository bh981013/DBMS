select p.name, c.level, c.nickname
from Pokemon p, CatchedPokemon c
where p.id = c.pid and c.owner_id in (select leader_id from Gym) and c.nickname like 'A%'
order by name desc;